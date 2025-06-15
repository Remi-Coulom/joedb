#ifndef joedb_Websocket_Channel_declared
#define joedb_Websocket_Channel_declared

#include "joedb/concurrency/Channel.h"

#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>

namespace joedb
{
 class Websocket_Channel: public Channel
 {
  private:
   boost::asio::io_context io_context;
   boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws;

  public:
   Websocket_Channel
   (
    const std::string &host,
    const std::string &port,
    const std::string &path
   ):
    ws(io_context)
   {
    const auto endpoint = boost::asio::connect
    (
     ws.next_layer(),
     boost::asio::ip::tcp::resolver(io_context).resolve(host, port)
    );

    ws.handshake
    (
     host + ":" + std::to_string(endpoint.port()),
     path
    );
   }

   size_t write_some(const char *data, size_t size) override
   {
    return ws.write(boost::asio::buffer(data, size));
   }

   size_t read_some(char *data, size_t size) override
   {
    return ws.read_some(boost::asio::buffer(data, size));
   }
  
   ~Websocket_Channel()
   {
    boost::system::error_code ec;
    ws.close(boost::beast::websocket::close_code::normal, ec);
   }
 };
}

#endif
