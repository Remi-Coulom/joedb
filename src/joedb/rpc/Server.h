#ifndef joedb_rpc_Server_declared
#define joedb_rpc_Server_declared

#include <boost/asio/io_context.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/signal_set.hpp>
#include "boost/asio/awaitable.hpp"
#include "boost/asio/detached.hpp"
#include "boost/asio/co_spawn.hpp"
#include "boost/asio/write.hpp"
#include "boost/asio/strand.hpp"
#include "boost/asio/bind_executor.hpp"

namespace joedb::rpc
{
 /// RPC Server
 ///
 /// @ingroup RPC
 class Server
 {
  protected:
   const std::string endpoint_path;
   boost::asio::local::stream_protocol::endpoint endpoint;
   boost::asio::local::stream_protocol::acceptor acceptor;
   boost::asio::signal_set interrupt_signals;

   boost::asio::io_context io_context;
   boost::asio::strand<boost::asio::io_context::executor_type> main_strand;

   // TODO: add logging

   struct Session
   {
    const int64_t id;
    Server &server;
    boost::asio::local::stream_protocol::socket socket;
    boost::asio::strand<boost::asio::io_context::executor_type> strand;

    Session
    (
     int64_t id,
     Server &server,
     boost::asio::local::stream_protocol::socket &&socket
    ):
     id(id),
     server(server),
     socket(std::move(socket)),
     strand(server.io_context.get_executor())
    {
    }

    virtual boost::asio::awaitable<void> run() // session strand
    {
     std::array<char, 1024> buffer;

     while (true)
     {
      size_t n = co_await socket.async_read_some
      (
       boost::asio::buffer(buffer),
       boost::asio::use_awaitable
      );

      // TODO: log received n bytes

      co_await boost::asio::async_write
      (
       socket,
       boost::asio::buffer(buffer, n),
       boost::asio::use_awaitable
      );
     }
    }

    virtual ~Session() = default;
   };

   virtual std::unique_ptr<Session> new_session // main_strand
   (
    int64_t id,
    boost::asio::local::stream_protocol::socket &&socket
   )
   {
    return std::make_unique<Session>(id, *this, std::move(socket));
   }

  private:
   int64_t session_id = 0;

   boost::asio::awaitable<void> listener() // main_strand
   {
    while (true)
    {
     auto session = new_session
     (
      session_id++,
      co_await acceptor.async_accept(boost::asio::use_awaitable)
     );

     // TODO: log session creation

     const auto session_ptr = session.get();

     boost::asio::co_spawn
     (
      session_ptr->strand,
      session_ptr->run(),
      boost::asio::bind_executor
      (
       main_strand,
       [ending_session = std::move(session)](std::exception_ptr exception_ptr)
       {
        // TODO: log session destruction
       }
      )
     );
    }
   }

  public:
   Server(std::string endpoint_path):
    endpoint_path(std::move(endpoint_path)),
    endpoint(this->endpoint_path),
    acceptor(io_context, endpoint, false),
    interrupt_signals(io_context, SIGINT, SIGTERM),
    main_strand(io_context.get_executor())
   {
    // TODO: log server creation
    interrupt_signals.async_wait
    (
     boost::asio::bind_executor
     (
      main_strand,
      [this](const boost::system::error_code &error, int)
      {
       // TODO: log interruption
       this->io_context.stop();
      }
     )
    );

    boost::asio::co_spawn
    (
     main_strand,
     listener(),
     boost::asio::detached
    );
   }

   boost::asio::io_context &get_io_context()
   {
    return io_context;
   }

   virtual ~Server() = default;
 };
}

#endif
