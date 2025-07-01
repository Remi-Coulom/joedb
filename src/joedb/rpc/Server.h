#ifndef joedb_rpc_Server_declared
#define joedb_rpc_Server_declared

#include "joedb/asio/Server.h"

#include "boost/asio/awaitable.hpp"
#include "boost/asio/detached.hpp"
#include "boost/asio/co_spawn.hpp"
#include "boost/asio/write.hpp"
#include "boost/asio/strand.hpp"
#include "boost/asio/bind_executor.hpp"

#include <set>

namespace joedb::rpc
{
 /// RPC Server
 ///
 /// @ingroup RPC
 class Server: public joedb::asio::Server
 {
  private:
   boost::asio::strand<boost::asio::io_context::executor_type> main_strand;

   struct Session: public std::enable_shared_from_this<Session>
   {
    Server &server;
    boost::asio::local::stream_protocol::socket socket;
    boost::asio::strand<boost::asio::io_context::executor_type> strand;

    Session
    (
     Server &server,
     boost::asio::local::stream_protocol::socket &&socket
    ):
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

      co_await boost::asio::async_write
      (
       socket,
       boost::asio::buffer(buffer, n),
       boost::asio::use_awaitable
      );
     }
    }

    virtual void cancel()
    {
     boost::asio::post
     (
      strand,
      [session = shared_from_this()](){session->socket.close();}
     );
    }

    virtual ~Session() = default;
   };

   std::set<std::shared_ptr<Session>> sessions;

   virtual std::shared_ptr<Session> new_session
   (
    boost::asio::local::stream_protocol::socket &socket
   )
   {
    return std::make_shared<Session>(*this, std::move(socket));
   }

   boost::asio::awaitable<void> listener() // main_strand
   {
    while (true)
    {
     boost::asio::local::stream_protocol::socket socket =
      co_await acceptor.async_accept(boost::asio::use_awaitable);

     if (!stopped)
     {
      auto session_iterator = sessions.emplace(new_session(socket)).first;

      boost::asio::co_spawn
      (
       (*session_iterator)->strand,
       (*session_iterator)->run(),
       boost::asio::bind_executor
       (
        main_strand,
        [this, session_iterator](std::exception_ptr exception_ptr)
        {
         sessions.erase(session_iterator);
        }
       )
      );
     }
    }
   }

   void start() // main_strand
   {
    if (stopped)
    {
     stopped = false;

     interrupt_signals.async_wait
     (
      boost::asio::bind_executor
      (
       main_strand,
       [this](const boost::system::error_code &error, int)
       {
        stop();
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
   }

   void stop() // main_strand
   {
    if (!stopped)
    {
     stopped = true;

     interrupt_signals.cancel();
     acceptor.cancel();

     for (const auto &session: sessions)
      session->cancel();
    }
   }

  public:
   Server
   (
    boost::asio::io_context &io_context,
    std::string endpoint_path
   ):
    joedb::asio::Server(io_context, endpoint_path),
    main_strand(io_context.get_executor())
   {
    async_start();
   }

   void async_start()
   {
    boost::asio::post(main_strand, [this](){start();});
   }

   void async_stop()
   {
    boost::asio::post(main_strand, [this](){stop();});
   }

   virtual ~Server() = default;
 };
}

#endif
