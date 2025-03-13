#ifndef joedb_Server_declared
#define joedb_Server_declared

#include "joedb/journal/Buffer.h"
#include "joedb/concurrency/net.h"
#include "joedb/concurrency/Client.h"
#include "joedb/io/Progress_Bar.h"

#include <queue>
#include <iosfwd>
#include <set>
#include <chrono>
#include <optional>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Server
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   enum {interrupt_check_seconds = 2};
   enum {clear_signal_seconds = 3};

   const std::chrono::time_point<std::chrono::steady_clock> start_time;
   Pullonly_Client &client;
   Client * const push_client;
   const bool share_client;
   std::optional<Client_Lock> client_lock;
   net::io_context &io_context;
   net::ip::tcp::acceptor acceptor;
   const uint16_t port;
   net::steady_timer interrupt_timer;
   bool paused;

   int64_t session_id;

   struct Session: public std::enable_shared_from_this<Session>
   {
    const int64_t id;
    Server &server;
    net::ip::tcp::socket socket;
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

    size_t push_remaining_size;
    char push_status;
    std::optional<Async_Writer> push_writer;
    bool unlock_after_push;

    std::optional<net::steady_timer> pull_timer;
    bool lock_before_pulling;
    int64_t pull_checkpoint;

    std::ostream &write_id(std::ostream &out) const;
    std::optional<io::Progress_Bar> progress_bar;

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
    size_t bytes_transferred,
    size_t offset
   );

   void start_pulling(std::shared_ptr<Session> session);

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

   template<typename F> void log(F f)
   {
    if (log_pointer)
    {
     f(*log_pointer);
     log_pointer->flush();
    }
   }

  public:
   Server
   (
    Pullonly_Client &client,
    bool share_client,
    net::io_context &io_context,
    uint16_t port,
    std::chrono::seconds lock_timeout,
    std::ostream *log_pointer
   );

   static constexpr int64_t server_version = 13;

   uint16_t get_port() const {return port;}
   bool is_readonly() const;

   void set_log(std::ostream *new_log);
   void pause();
   void restart();
   void send_signal(int status);
   std::chrono::milliseconds get_time_stamp() const;

   ~Server();
 };
}

#endif
