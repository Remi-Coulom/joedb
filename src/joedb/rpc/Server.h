#ifndef joedb_rpc_Server_declared
#define joedb_rpc_Server_declared

#include "joedb/error/Logger.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/signal_set.hpp>
#include "boost/asio/awaitable.hpp"
#include "boost/asio/detached.hpp"
#include "boost/asio/co_spawn.hpp"
#include "boost/asio/write.hpp"
#include "boost/asio/strand.hpp"
#include "boost/asio/bind_executor.hpp"

#include <thread>

namespace joedb::rpc
{
 /// RPC Server
 ///
 /// @ingroup RPC
 class Server
 {
  protected:
   Logger &logger;
   const int log_level;
   void log(std::string_view s)
   {
    logger.write(endpoint_path + ": " + std::string(s) + '\n');
   }

   const int thread_count;
   boost::asio::io_context io_context;
   boost::asio::strand<boost::asio::io_context::executor_type> main_strand;

   const std::string endpoint_path;
   boost::asio::local::stream_protocol::endpoint endpoint;
   boost::asio::local::stream_protocol::acceptor acceptor;
   boost::asio::signal_set interrupt_signals;

   struct Session
   {
    const int64_t id;
    Server &server;
    boost::asio::local::stream_protocol::socket socket;
    const boost::asio::strand<boost::asio::io_context::executor_type> strand;

    void log(std::string_view s)
    {
     server.log(std::to_string(id) + ": " + std::string(s));
    }

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
     if (server.log_level > 1)
      log("start");
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

      if (server.log_level > 2)
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

     const auto session_ptr = session.get();

     boost::asio::co_spawn
     (
      session_ptr->strand,
      session_ptr->run(),
      boost::asio::bind_executor
      (
       main_strand,
       [ending_session = std::move(session), this]
       (
        std::exception_ptr exception_ptr
       )
       {
        if (log_level > 1)
         ending_session->log("stop");
       }
      )
     );
    }
   }

  public:
   Server
   (
    Logger &logger,
    int log_level,
    int thread_count,
    std::string endpoint_path
   ):
    logger(logger),
    log_level(log_level),
    thread_count(thread_count),
    io_context(thread_count),
    main_strand(io_context.get_executor()),
    endpoint_path(std::move(endpoint_path)),
    endpoint(this->endpoint_path),
    acceptor(io_context, endpoint, false),
    interrupt_signals(io_context, SIGINT, SIGTERM)
   {
    if (log_level > 0)
     log("start");

    interrupt_signals.async_wait
    (
     boost::asio::bind_executor
     (
      main_strand,
      [this](const boost::system::error_code &error, int)
      {
       if (this->log_level > 0)
        this->log("interrupting...");
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

   void run()
   {
    log("run, thread_count = " + std::to_string(thread_count));
    std::vector<std::thread> threads;
    threads.reserve(thread_count);

    for (int i = thread_count; --i >= 0;)
    {
     threads.emplace_back
     (
      [this]()
      {
       try
       {
        io_context.run();
       }
       catch (...)
       {
       }
      }
     );
    }

    for (auto &thread: threads)
     thread.join();

    log("stop");
   }

   virtual ~Server()
   {
    try
    {
     std::remove(endpoint_path.c_str());
    }
    catch (...)
    {
    }
   }
 };
}

#endif
