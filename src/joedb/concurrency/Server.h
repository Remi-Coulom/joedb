#ifndef joedb_Server_declared
#define joedb_Server_declared

#include "joedb/asio/Server.h"
#include "joedb/journal/Buffer.h"
#include "joedb/concurrency/Writable_Journal_Client.h"

#include <boost/asio/experimental/channel.hpp>

#include <queue>
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
    public:
     const Server &get_server() const {return *(Server *)&server;}
     Server &get_server() {return *(Server *)&server;}

     boost::asio::experimental::channel<void(boost::system::error_code)> channel;
     bool locking = false;

    public:
     Buffer<13> buffer;

     Session
     (
      Server &server,
      boost::asio::local::stream_protocol::socket &&socket
     );

     void unlock();

     boost::asio::awaitable<size_t> read_buffer(size_t offset, size_t size);
     boost::asio::awaitable<void> write_buffer();
     boost::asio::awaitable<void> send(Async_Reader reader);
     boost::asio::awaitable<void> handshake();
     boost::asio::awaitable<void> check_hash();
     boost::asio::awaitable<void> read();
     boost::asio::awaitable<void> pull(bool lock_before, bool send_data);
     boost::asio::awaitable<void> push(bool unlock_after);
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

   boost::asio::awaitable<void> lock(Session &session);

   const std::chrono::milliseconds lock_timeout;
   boost::asio::steady_timer lock_timeout_timer;

   bool locked;
   std::queue<Session *> lock_waiters;
   std::queue<Session *> pull_waiters;

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
