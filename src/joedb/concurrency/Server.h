#ifndef joedb_Server_declared
#define joedb_Server_declared

#include "joedb/journal/Writable_Journal.h"
#include "joedb/concurrency/net.h"

#include <queue>
#include <memory>
#include <iosfwd>
#include <set>

namespace joedb
{
 class Backup_Client;

 ////////////////////////////////////////////////////////////////////////////
 class Server
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   enum {interrupt_check_seconds = 2};
   enum {clear_signal_seconds = 3};

   joedb::Writable_Journal &journal;
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
    enum {buffer_size = (1 << 13)};
    char buffer[buffer_size];
    enum State
    {
     not_locking,
     waiting_for_lock,
     waiting_for_lock_pull,
     locking
    };
    bool unlock_after_push;
    State state;

    std::ostream &write_id(std::ostream &out) const;

    Session(Server &server, net::ip::tcp::socket &&socket);
    ~Session();
   };

   std::set<Session *> sessions;

   void write_status();

   const uint32_t lock_timeout_seconds;
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
   std::vector<char> push_buffer;

   void push_transfer_handler
   (
    std::shared_ptr<Session> session,
    size_t offset,
    size_t remaining_size,
    bool conflict,
    std::error_code error,
    size_t bytes_transferred
   );

   void push_transfer
   (
    std::shared_ptr<Session> session,
    size_t offset,
    size_t remaining_size,
    bool conflict
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
    joedb::Writable_Journal &journal,
    net::io_context &io_context,
    uint16_t port,
    uint32_t lock_timeout_seconds,
    std::ostream *log_pointer
   );

   uint16_t get_port() const {return port;}
   static void interrupt();

   ~Server();
 };
}

#endif
