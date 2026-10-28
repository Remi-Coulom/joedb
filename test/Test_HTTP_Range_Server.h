#ifndef joedb_Test_HTTP_Range_Server_declared
#define joedb_Test_HTTP_Range_Server_declared

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/streambuf.hpp>
#include <algorithm>
#include <cctype>
#include <memory>
#include <sstream>
#include <thread>

namespace joedb
{
 class Test_HTTP_Range_Server
 {
  private:
   using Socket = boost::asio::ip::tcp::socket;
   boost::asio::io_context context;
   boost::asio::ip::tcp::acceptor acceptor
    {context, {boost::asio::ip::address_v4::loopback(), 0}};
   std::thread thread;

   static std::string reply(std::string request)
   {
    const std::string body = "0123456789";
    std::transform(request.begin(), request.end(), request.begin(),
     [](unsigned char c) {return char(std::tolower(c));});
    const std::string prefix = "\r\nrange: bytes=";
    const size_t header = request.find(prefix);
    std::string status = "200 OK";
    std::string payload = body;
    std::string content_range;
    if (header != std::string::npos)
    {
     const size_t start = header + prefix.size();
     const std::string range = request.substr(start, request.find("\r\n", start) - start);
     const size_t dash = range.find('-');
     const auto digits = [](const std::string &s)
     {
      return !s.empty() && std::all_of(s.begin(), s.end(),
       [](unsigned char c) {return c >= '0' && c <= '9';});
     };
     const std::string first = range.substr(0, dash);
     const std::string last = dash == std::string::npos ? "" : range.substr(dash + 1);
     if (dash == std::string::npos || !digits(first) || (!last.empty() && !digits(last)))
     {
      status = "400 Bad Request";
      payload.clear();
     }
     else
     {
      const size_t from = size_t(std::stoull(first));
      const size_t until = last.empty() ? body.size() - 1 : size_t(std::stoull(last));
      if (from >= body.size() || until < from)
      {
       status = "416 Range Not Satisfiable";
       payload.clear();
       content_range = "Content-Range: bytes */10\r\n";
      }
      else
      {
       const size_t end = std::min(until, body.size() - 1);
       status = "206 Partial Content";
       payload = body.substr(from, end - from + 1);
       content_range = "Content-Range: bytes " + std::to_string(from) + '-' +
        std::to_string(end) + "/10\r\n";
      }
     }
    }
    return "HTTP/1.1 " + status + "\r\nConnection: close\r\n" + content_range +
     "Content-Length: " + std::to_string(payload.size()) + "\r\n\r\n" + payload;
   }

   struct Session: std::enable_shared_from_this<Session>
   {
    Socket socket;
    boost::asio::streambuf request;
    std::string response;
    explicit Session(Socket socket): socket(std::move(socket)) {}
    void start()
    {
     boost::asio::async_read_until(socket, request, "\r\n\r\n",
      [self = shared_from_this()](const boost::system::error_code &error, size_t)
      {
       if (error)
        return;
       std::ostringstream text;
       text << &self->request;
       self->response = reply(text.str());
       boost::asio::async_write(self->socket, boost::asio::buffer(self->response),
        [self](const boost::system::error_code &, size_t)
        {
         boost::system::error_code ignored;
         self->socket.close(ignored);
        });
      });
    }
   };

   void accept()
   {
    acceptor.async_accept([this](const boost::system::error_code &error, Socket socket)
    {
     if (!error)
     {
      std::make_shared<Session>(std::move(socket))->start();
      accept();
     }
    });
   }

  public:
   Test_HTTP_Range_Server()
   {
    accept();
    thread = std::thread([this]() {context.run();});
   }
   std::string url() const
   {
    return "http://127.0.0.1:" + std::to_string(acceptor.local_endpoint().port()) + "/data";
   }
   ~Test_HTTP_Range_Server()
   {
    context.stop();
    thread.join();
   }
 };
}

#endif
