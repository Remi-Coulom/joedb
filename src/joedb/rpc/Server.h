#ifndef joedb_rpc_Server_declared
#define joedb_rpc_Server_declared

#include "joedb/asio/Server.h"

#include <boost/asio/write.hpp>
#include <boost/asio/co_spawn.hpp>

#include <thread>

namespace joedb::rpc
{
 /// RPC Server
 ///
 /// @ingroup RPC
 class Server: public joedb::asio::Server
 {
  protected:
   class Session: public joedb::asio::Server::Session
   {
    public:
     Session
     (
      Server &server,
      boost::asio::local::stream_protocol::socket &&socket
     ):
      joedb::asio::Server::Session(server, std::move(socket))
     {
     }

     boost::asio::awaitable<void> run() override
     {
      std::array<char, 1024> buffer;

      while (true)
      {
       size_t n = co_await socket.async_read_some
       (
        boost::asio::buffer(buffer),
        boost::asio::use_awaitable
       );

       if (server.get_log_level() > 2)
        log(std::to_string(n) + " bytes");

       std::this_thread::sleep_for(std::chrono::seconds(10));

       co_await boost::asio::async_write
       (
        socket,
        boost::asio::buffer(buffer, n),
        boost::asio::use_awaitable
       );
      }
     }
   };

   std::unique_ptr<joedb::asio::Server::Session> new_session
   (
    boost::asio::local::stream_protocol::socket &&socket
   ) override
   {
    return std::make_unique<Session>(*this, std::move(socket));
   }

  public:
   Server
   (
    Logger &logger,
    int log_level,
    int thread_count,
    std::string endpoint_path
   ):
    joedb::asio::Server
    (
     logger,
     log_level,
     thread_count,
     std::move(endpoint_path)
    )
   {
   }
 };
}

#endif
