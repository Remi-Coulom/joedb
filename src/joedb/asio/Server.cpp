#include "joedb/asio/Server.h"
#include "joedb/ui/get_time_string.h"

#include <boost/asio/detached.hpp>
#include <boost/asio/co_spawn.hpp>

namespace joedb::asio
{
 void Server::log(std::string_view s)
 {
  logger.write(joedb::get_time_string_of_now() + ' ' + endpoint_path + ": " + std::string(s) + '\n');
 }

 Server::Session::Session
 (
  Server &server,
  boost::asio::local::stream_protocol::socket &&socket
 ):
  server(server),
  id(server.session_id++),
  socket(std::move(socket)),
  strand(server.thread_pool.get_executor())
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
    session_ptr->strand,
    session_ptr->run(),
    [ending_session = std::move(session), this]
    (
     std::exception_ptr exception_ptr
    )
    {
     ending_session->cleanup();
     if (log_level > 1)
      ending_session->log("stop");
     if (exception_ptr && log_level > 1)
     {
      try
      {
       std::rethrow_exception(exception_ptr);
      }
      catch (const std::exception &e)
      {
       if (log_level > 1)
        ending_session->log(std::string("exception: ") + e.what());
      }
      catch (...)
      {
      }
     }
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
  thread_pool(thread_count),
  endpoint_path(std::move(endpoint_path)),
  endpoint(this->endpoint_path),
  acceptor(thread_pool, endpoint, false),
  interrupt_signals(thread_pool, SIGINT, SIGTERM)
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
     this->thread_pool.stop();
    }
   }
  );

  boost::asio::co_spawn
  (
   thread_pool,
   listener(),
   boost::asio::detached
  );
 }

 void Server::run()
 {
  if (log_level > 0)
   log("run, thread_count = " + std::to_string(thread_count));

  thread_pool.join();

  if (log_level > 0)
   log("stop");
 }

 void Server::stop()
 {
  thread_pool.stop();
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
