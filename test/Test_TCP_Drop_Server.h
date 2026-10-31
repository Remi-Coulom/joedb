#ifndef joedb_Test_TCP_Drop_Server_declared
#define joedb_Test_TCP_Drop_Server_declared

#include <boost/asio/ip/tcp.hpp>
#include <thread>
#include <atomic>

namespace joedb
{
 class Test_TCP_Drop_Server
 {
  private:
   boost::asio::io_context context;
   boost::asio::ip::tcp::acceptor acceptor
   {
    context,
    {boost::asio::ip::address_v4::loopback(), 0}
   };
   std::thread thread;
   std::atomic<bool> accepted{false};

  public:
   Test_TCP_Drop_Server()
   {
    acceptor.async_accept([this](const boost::system::error_code &error, boost::asio::ip::tcp::socket)
    {
     accepted = !error;
    });
    thread = std::thread([this]() {context.run();});
   }

   unsigned port() const {return acceptor.local_endpoint().port();}
   bool accepted_connection() const {return accepted;}

   ~Test_TCP_Drop_Server()
   {
    context.stop();
    thread.join();
   }
 };
}

#endif
