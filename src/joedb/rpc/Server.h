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

   // TODO: Thread_Safe_Sockets wrapper (make a generic template)
   std::mutex sockets_mutex;
   std::set<boost::asio::local::stream_protocol::socket *> sockets;

   class Socket_Tracker
   {
    private:
     Server &server;
     boost::asio::local::stream_protocol::socket &socket;

    public:
     Socket_Tracker
     (
      Server &server,
      boost::asio::local::stream_protocol::socket &socket
     ): server(server), socket(socket)
     {
      std::unique_lock lock(server.sockets_mutex);
      server.sockets.insert(&socket);
     }

     ~Socket_Tracker()
     {
      std::unique_lock lock(server.sockets_mutex);
      server.sockets.erase(&socket);
     }
   };

   boost::asio::awaitable<void> session
   (
    boost::asio::local::stream_protocol::socket socket
   )
   {
    Socket_Tracker socket_tracker(*this, socket);

    std::array<char, 1024> buffer;

    while (true)
    {
     size_t n = co_await socket.async_read_some
     (
      boost::asio::buffer(buffer),
      boost::asio::use_awaitable
     );

     if (n < procedures.size())
     {
      co_await boost::asio::async_write
      (
       socket,
       boost::asio::buffer(buffer, n),
       boost::asio::use_awaitable
      );
     }
    }
   }

   boost::asio::awaitable<void> listener()
   {
    while (true)
    {
     boost::asio::local::stream_protocol::socket socket =
      co_await acceptor.async_accept(boost::asio::use_awaitable);

     if (!stopped)
     {
      boost::asio::co_spawn
      (
       io_context,
       session(std::move(socket)),
       boost::asio::detached
      );
     }
    }
   }

   void start()
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

   void stop()
   {
    if (!stopped)
    {
     stopped = true;

     interrupt_signals.cancel();
     acceptor.cancel();

     {
      std::unique_lock lock(sockets_mutex);
      for (auto *socket: sockets)
       socket->close();
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
