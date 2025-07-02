#include "joedb/asio/Server.h"

#include <boost/asio/detached.hpp>
#include <boost/asio/co_spawn.hpp>

#include <thread>

namespace joedb::asio
{
 void Server::log(std::string_view s)
 {
  logger.write(endpoint_path + ": " + std::string(s) + '\n');
 }

 Server::Session::Session
 (
  Server &server,
  boost::asio::local::stream_protocol::socket &&socket
 ):
  id(server.session_id++),
  server(server),
  socket(std::move(socket))
 {
  if (server.log_level > 1)
   log("start");
 }

 void Server::Session::log(std::string_view s)
 {
  server.log(std::to_string(id) + ": " + std::string(s));
 }

 Server::Session::~Session() = default;

 boost::asio::awaitable<void> Server::listener()
 {
  while (true)
  {
   auto session = new_session
   (
    co_await acceptor.async_accept(boost::asio::use_awaitable)
   );

   const auto session_ptr = session.get();

   boost::asio::co_spawn
   (
    io_context,
    session_ptr->run(),
    [ending_session = std::move(session), this]
    (
     std::exception_ptr exception_ptr
    )
    {
     if (log_level > 1)
      ending_session->log("stop");
    }
   );
  }
 }

 Server::Server
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
  endpoint_path(std::move(endpoint_path)),
  endpoint(this->endpoint_path),
  acceptor(io_context, endpoint, false),
  interrupt_signals(io_context, SIGINT, SIGTERM)
 {
  if (log_level > 0)
   log("start");

  interrupt_signals.async_wait
  (
   [this](const boost::system::error_code &error, int signal)
   {
    if (!error)
    {
     if (this->log_level > 0)
     {
      if (signal == SIGINT)
       this->log("interrupted by SIGINT");
      else if (signal == SIGTERM)
       this->log("interrupted by SIGTERM");
     }
     this->io_context.stop();
    }
   }
  );

  boost::asio::co_spawn
  (
   io_context,
   listener(),
   boost::asio::detached
  );
 }

 void Server::run()
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

 Server::~Server()
 {
  try
  {
   std::remove(endpoint_path.c_str());
  }
  catch (...)
  {
  }
 }
}
