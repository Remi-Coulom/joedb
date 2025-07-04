#ifndef joedb_asio_Server_declared
#define joedb_asio_Server_declared

#include "joedb/error/Logger.h"

#include <boost/asio/thread_pool.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/awaitable.hpp>

namespace joedb::asio
{
 /// Superclass for asio servers
 ///
 /// @ingroup RPC
 class Server
 {
  protected:
   Logger &logger;
   const int log_level;
   void log(std::string_view s);

   const int thread_count;
   boost::asio::thread_pool thread_pool;

   const std::string endpoint_path;
   boost::asio::local::stream_protocol::endpoint endpoint;
   boost::asio::local::stream_protocol::acceptor acceptor;
   boost::asio::signal_set interrupt_signals;

   class Session
   {
    public: // TODO -> protected
     const int64_t id;
     Server &server;
     boost::asio::local::stream_protocol::socket socket;

    public:
     Session
     (
      Server &server,
      boost::asio::local::stream_protocol::socket &&socket
     );

     void log(std::string_view s);

     virtual boost::asio::awaitable<void> run() = 0;
     virtual void cleanup() {}

     virtual ~Session();
   };

   virtual std::unique_ptr<Session> new_session
   (
    boost::asio::local::stream_protocol::socket &&socket
   ) = 0;

  private:
   int64_t session_id = 0;

   boost::asio::awaitable<void> listener();

  public:
   Server
   (
    Logger &logger,
    int log_level,
    int thread_count,
    std::string endpoint_path
   );

   int get_log_level() const
   {
    return log_level;
   }

   const std::string &get_endpoint_path() const
   {
    return endpoint_path;
   }

   void run();

   virtual ~Server();
 };
}

#endif
