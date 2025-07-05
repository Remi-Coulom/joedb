#ifndef joedb_Server_declared
#define joedb_Server_declared

#include "joedb/asio/Server.h"
#include "joedb/concurrency/Writable_Journal_Client.h"

#include <deque>
#include <map>

namespace joedb
{
 /// @ingroup concurrency
 class Server: public joedb::asio::Server
 {
  private:
   Client &client;
   Writable_Journal_Client *writable_journal_client;
   std::optional<Writable_Journal_Client_Lock> client_lock;

   class Session: public joedb::asio::Server::Session
   {
    private:
     const Server &get_server() const {return *(Server *)&server;}
     Server &get_server() {return *(Server *)&server;}

     boost::asio::steady_timer timer;
     bool locking = false;

     boost::asio::steady_timer lock_timeout_timer;
     boost::asio::awaitable<void> lock();
     void unlock();
     void refresh_lock_timeout();

     char push_status;
     std::optional<joedb::Async_Writer> push_writer;

     boost::asio::awaitable<void> send(Async_Reader reader);
     boost::asio::awaitable<void> handshake();
     boost::asio::awaitable<void> check_hash();
     boost::asio::awaitable<void> read();
     boost::asio::awaitable<void> pull(bool lock_before, bool send_data);
     boost::asio::awaitable<void> push(bool unlock_after);

    public:
     Session
     (
      Server &server,
      boost::asio::local::stream_protocol::socket &&socket
     );

     boost::asio::awaitable<void> run() override;
     void cleanup() override;
   };

   std::unique_ptr<joedb::asio::Server::Session> new_session
   (
    boost::asio::local::stream_protocol::socket &&socket
   ) override
   {
    return std::make_unique<Session>(*this, std::move(socket));
   }

   const std::chrono::milliseconds lock_timeout;

   bool locked;
   std::deque<Session *> lock_waiters;
   std::deque<Session *> pull_waiters;

   static const std::map<char, const char *> request_description;

  public:
   Server
   (
    Logger &logger,
    int log_level,
    int thread_count,
    std::string endpoint_path,
    Client &client,
    std::chrono::milliseconds lock_timeout
   );

   bool has_client_lock() const {return bool(client_lock);}

   ~Server();
 };
}

#endif
