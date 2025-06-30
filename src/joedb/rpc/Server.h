#ifndef joedb_rpc_Server_declared
#define joedb_rpc_Server_declared

#include "joedb/rpc/Procedure.h"
#include "joedb/asio/Server.h"

#include "boost/asio/awaitable.hpp"
#include "boost/asio/detached.hpp"
#include "boost/asio/co_spawn.hpp"
#include "boost/asio/write.hpp"
#include "boost/asio/strand.hpp"

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
     std::lock_guard lock(server.sessions_mutex);
     server.sessions.insert(this);
    }
   };

   std::mutex sessions_mutex;
   std::set<Session *> sessions;

   boost::asio::awaitable<void> run_session // session->strand
   (
    std::unique_ptr<Session> session
   )
   {
    try
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
    catch (...)
    {
    }

    {
     std::lock_guard lock(sessions_mutex);
     sessions.erase(session.get());
    }

    // TODO: completion handler
    // https://stackoverflow.com/questions/68041906/boostasioco-spawn-does-not-propagate-exception
    // -> stop the server in case of exception
   }

   boost::asio::awaitable<void> listener() // main_strand
   {
    while (true)
    {
     boost::asio::local::stream_protocol::socket socket =
      co_await acceptor.async_accept(boost::asio::use_awaitable);

     if (!stopped)
     {
      auto session = std::make_unique<Session>(*this, std::move(socket));
      boost::asio::co_spawn
      (
       session->strand,
       run_session(std::move(session)),
       boost::asio::detached
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
      [this](const boost::system::error_code &error, int)
      {
       if (!error)
        async_stop();
      }
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

     {
      std::lock_guard lock(sessions_mutex);
      for (auto *session: sessions)
      {
       boost::asio::post
       (
        session->strand,
        [session](){session->socket.close();}
       );
      }
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
