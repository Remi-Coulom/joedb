#ifndef joedb_Server_declared
#define joedb_Server_declared

#include "joedb/journal/Writable_Journal.h"
#include "joedb/concurrency/net.h"
#include "joedb/concurrency/Client.h"

#include <queue>
#include <memory>
#include <iosfwd>
#include <set>
#include <chrono>
#include <array>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Server
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   enum {interrupt_check_seconds = 2};
   enum {clear_signal_seconds = 3};

   joedb::Client &client;
   const bool share_client;
   std::unique_ptr<Client_Lock> client_lock;
   net::io_context &io_context;
   net::ip::tcp::acceptor acceptor;
   const uint16_t port;
   net::steady_timer interrupt_timer;

   int64_t session_count;
   int64_t session_id;

   struct Session
   {
    const int64_t id;
    Server &server;
    net::ip::tcp::socket socket;
    std::array<char, 1 << 13> buffer;
    enum State
    {
     not_locking,
     waiting_for_lock,
     waiting_for_lock_pull,
     locking
    };
    State state;

    size_t push_remaining_size;
    char push_status;
    std::unique_ptr<Journal_Tail_Writer> push_writer;
    bool unlock_after_push;

    std::ostream &write_id(std::ostream &out) const;

    Session(Server &server, net::ip::tcp::socket &&socket);
    ~Session();
   };

   std::set<Session *> sessions;

   void write_status();

   const std::chrono::seconds lock_timeout;
   net::steady_timer lock_timeout_timer;
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

   void pull_transfer_handler
   (
    std::shared_ptr<Session> session,
    Async_Reader reader,
    std::error_code error,
    size_t bytes_transferred
   );

   void pull_handler
   (
    std::shared_ptr<Session> session,
    std::error_code error,
    size_t bytes_transferred
   );

   void pull(std::shared_ptr<Session> session);

   void check_hash_handler
   (
    std::shared_ptr<Session> session,
    std::error_code error,
    size_t bytes_transferred
   );

   void check_hash(std::shared_ptr<Session> session);

   void read_command_handler
   (
    std::shared_ptr<Session> session,
    std::error_code error,
    size_t bytes_transferred
   );

   void read_command(std::shared_ptr<Session> session);

   void write_buffer_and_next_command_handler
   (
    std::shared_ptr<Session> session,
    std::error_code error,
    size_t bytes_transferred
   );

   void write_buffer_and_next_command
   (
    std::shared_ptr<Session> session,
    size_t size
   );

   void handshake(std::shared_ptr<Session> session);

   void handshake_handler
   (
    std::shared_ptr<Session> session,
    std::error_code error,
    size_t bytes_transferred
   );

   void handle_accept
   (
    std::error_code error,
    net::ip::tcp::socket socket
   );

   void start_accept();

   void start_interrupt_timer();
   void handle_interrupt_timer(std::error_code error);
   void handle_clear_signal_timer(std::error_code error);

   std::ostream *log_pointer;

   template<typename F> void log(F f) noexcept
   {
    if (log_pointer)
    {
     try
     {
      f(*log_pointer);
      log_pointer->flush();
     }
     catch (...)
     {
     }
    }
   }

  public:
   Server
   (
    joedb::Client &client,
    bool share_client,
    net::io_context &io_context,
    uint16_t port,
    std::chrono::seconds lock_timeout,
    std::ostream *log_pointer
   );

   void set_log(std::ostream *new_log);

   uint16_t get_port() const {return port;}
   void interrupt();

   ~Server();
 };
}

#endif
