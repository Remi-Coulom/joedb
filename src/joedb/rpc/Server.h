#ifndef joedb_rpc_Server_declared
#define joedb_rpc_Server_declared

#include "joedb/rpc/Procedure.h"
#include "joedb/asio/Server.h"

#include "boost/asio/awaitable.hpp"
#include "boost/asio/detached.hpp"
#include "boost/asio/co_spawn.hpp"
#include "boost/asio/write.hpp"
#include "boost/asio/strand.hpp"
#include "boost/asio/bind_executor.hpp"

#include <vector>
#include <functional>
#include <set>

namespace joedb::rpc
{
 /// RPC Server
 ///
 /// @ingroup RPC
 class Server: public joedb::asio::Server
 {
  private:
   const std::vector<std::reference_wrapper<Procedure>> &procedures;

   boost::asio::strand<boost::asio::io_context::executor_type> main_strand;

   struct Session
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
   };

   std::set<std::shared_ptr<Session>> sessions;

   boost::asio::awaitable<void> run_session // session->strand
   (
    std::shared_ptr<Session> session
   )
   {
    std::array<char, 1024> buffer;

    while (true)
    {
     size_t n = co_await session->socket.async_read_some
     (
      boost::asio::buffer(buffer),
      boost::asio::use_awaitable
     );

     if (n < procedures.size())
     {
      co_await boost::asio::async_write
      (
       session->socket,
       boost::asio::buffer(buffer, n),
       boost::asio::use_awaitable
      );
     }
    }
   }

   boost::asio::awaitable<void> listener() // main_strand
   {
    while (true)
    {
     boost::asio::local::stream_protocol::socket socket =
      co_await acceptor.async_accept(boost::asio::use_awaitable);

     if (!stopped)
     {
      auto session_iterator = sessions.emplace
      (
       new Session(*this, std::move(socket))
      ).first;

      boost::asio::co_spawn
      (
       (*session_iterator)->strand,
       run_session(*session_iterator),
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

     for (auto &session: sessions)
     {
      boost::asio::post
      (
       session->strand,
       [session](){session->socket.close();}
      );
     }
    }
   }

  public:
   Server
   (
    const std::vector<std::reference_wrapper<Procedure>> &procedures,
    boost::asio::io_context &io_context,
    std::string endpoint_path
   ):
    joedb::asio::Server(io_context, endpoint_path),
    procedures(procedures),
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
 };
}

#endif
