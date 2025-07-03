#ifndef joedb_Server_declared
#define joedb_Server_declared

#include "joedb/asio/Server.h"
#include "joedb/journal/Buffer.h"
#include "joedb/concurrency/Writable_Journal_Client.h"

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
    private:
     const Server &get_server() const {return *(Server *)&server;}
     Server &get_server() {return *(Server *)&server;}

    public:
     Buffer<13> buffer;

     enum class State
     {
      not_locking,
      waiting_for_push_to_pull,
      waiting_for_lock_to_pull,
      waiting_for_lock_to_push,
      locking
     };
     State state;

     std::ostream &write_id(std::ostream &out) const;

     Session
     (
      Server &server,
      boost::asio::local::stream_protocol::socket &&socket
     );

     boost::asio::awaitable<void> read_buffer(size_t offset, size_t size);
     boost::asio::awaitable<void> write_buffer();
     boost::asio::awaitable<void> send(Async_Reader reader);
     boost::asio::awaitable<void> handshake();
     boost::asio::awaitable<void> check_hash();
     boost::asio::awaitable<void> pull(bool lock_before, bool send_data);
     boost::asio::awaitable<void> push(bool unlock_after);
     boost::asio::awaitable<void> run() override;

     ~Session();
   };

   std::unique_ptr<joedb::asio::Server::Session> new_session
   (
    boost::asio::local::stream_protocol::socket &&socket
   ) override
   {
    return std::make_unique<Session>(*this, std::move(socket));
   }

   std::vector<std::shared_ptr<Session>> waiting_sessions;

   typedef void (Server::*Transfer_Handler)
   (
    std::shared_ptr<Session> session,
    std::error_code error,
    size_t bytes_transferred
   );

   void async_read
   (
    std::shared_ptr<Session> session,
    size_t offset,
    size_t size,
    Transfer_Handler handler
   );

   const std::chrono::milliseconds lock_timeout;
   boost::asio::steady_timer lock_timeout_timer;
   bool locked;
   std::queue<std::shared_ptr<Session>> lock_queue;
   void lock_dequeue();
   void lock(std::shared_ptr<Session> session, Session::State state);
   void unlock(Session &session);

   void lock_timeout_handler
   (
    std::shared_ptr<Session> session,
    std::error_code error
   );

   void refresh_lock_timeout(std::shared_ptr<Session> session);

   void push_transfer_handler
   (
    std::shared_ptr<Session> session,
    std::error_code error,
    size_t bytes_transferred
   );

   void push_transfer
   (
    std::shared_ptr<Session> session
   );

   void push_handler
   (
    std::shared_ptr<Session> session,
    std::error_code error,
    size_t bytes_transferred
   );

   void read_transfer_handler
   (
    std::shared_ptr<Session> session,
    Async_Reader reader,
    std::error_code error,
    size_t bytes_transferred,
    size_t offset
   );

   void start_reading(std::shared_ptr<Session> session, Async_Reader reader);

   void start_pulling(std::shared_ptr<Session> session);

   void pull_handler
   (
    std::shared_ptr<Session> session,
    std::error_code error,
    size_t bytes_transferred
   );

   void read_handler
   (
    std::shared_ptr<Session> session,
    std::error_code error,
    size_t bytes_transferred
   );

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
 };
}

#endif
