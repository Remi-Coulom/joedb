#include "joedb/asio/Server.h"
#include "joedb/error/Destructor_Logger.h"

#include <boost/asio/detached.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

namespace joedb::asio
{
 void Server::log(std::string_view s)
 {
  logger.write(endpoint_path + ": " + std::string(s));
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

 boost::asio::awaitable<size_t> Server::Session::read_buffer
 (
  const size_t offset,
  const size_t size
 )
 {
  const size_t result = co_await boost::asio::async_read
  (
   socket,
   boost::asio::buffer(buffer.data + offset, size),
   boost::asio::use_awaitable
  );

  buffer.index = offset;

  co_return result;
 }

 boost::asio::awaitable<void> Server::Session::write_buffer()
 {
  co_await boost::asio::async_write
  (
   socket,
   boost::asio::buffer(buffer.data, buffer.index),
   boost::asio::use_awaitable
  );
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

   auto* const session_ptr = session.get();

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
  joined(false),
  endpoint_path(std::move(endpoint_path)),
  endpoint(this->endpoint_path),
  acceptor(thread_pool, endpoint, false),
  interrupt_signals(thread_pool, SIGINT, SIGTERM)
 {
  if (log_level > 0)
   log("start, thread_count = " + std::to_string(thread_count));

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

 void Server::stop()
 {
  thread_pool.stop();
  join();
 }

 void Server::join()
 {
  thread_pool.join();
  cleanup_after_join();
  std::remove(endpoint_path.c_str());
  joined = true;
 }

 void Server::cleanup_after_join()
 {
 }

 Server::~Server()
 {
  if (!joined)
  {
   Destructor_Logger::warning("Server: not joined. This is a bug.");
   std::terminate(); // it is too late to call join, since child was destroyed already
  }
 }
}
