#ifndef joedb_Server_declared
#define joedb_Server_declared

#include "joedb/journal/Buffer.h"
#include "joedb/concurrency/Client.h"
#include "joedb/ui/Progress_Bar.h"

#include <queue>
#include <iosfwd>
#include <set>
#include <chrono>
#include <optional>

#include <asio/ip/tcp.hpp>
#include <asio/steady_timer.hpp>
#include <asio/signal_set.hpp>

namespace joedb
{
 /// \ingroup concurrency
 class Server
 {
  private:
   const std::chrono::time_point<std::chrono::steady_clock> start_time;
   Pullonly_Client &client;
   Client * const push_client;
   const bool share_client;
   std::optional<Client_Lock> client_lock;
   asio::io_context &io_context;
   asio::ip::tcp::acceptor acceptor;
   const uint16_t port;
   bool stopped;
   asio::signal_set interrupt_signals;

   int64_t session_id;

   struct Session: public std::enable_shared_from_this<Session>
   {
    const int64_t id;
    Server &server;
    asio::ip::tcp::socket socket;
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

    std::optional<asio::steady_timer> pull_timer;
    bool lock_before_pulling;
    bool send_pull_data;
    int64_t pull_checkpoint;

    std::ostream &write_id(std::ostream &out) const;
    std::optional<Progress_Bar> progress_bar;

    Session(Server &server, asio::ip::tcp::socket &&socket);
    ~Session();
   };

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

   std::set<Session *> sessions;

   void write_status();

   const std::chrono::milliseconds lock_timeout;
   asio::steady_timer lock_timeout_timer;
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

   void pull(std::shared_ptr<Session> session, bool lock, bool send);

   void read_handler
   (
    std::shared_ptr<Session> session,
    std::error_code error,
    size_t bytes_transferred
   );

   void read_blob_handler
   (
    std::shared_ptr<Session> session,
    std::error_code error,
    size_t bytes_transferred
   );

   void check_hash_handler
   (
    std::shared_ptr<Session> session,
    std::error_code error,
    size_t bytes_transferred
   );

   void read_command_handler
   (
    std::shared_ptr<Session> session,
    std::error_code error,
    size_t bytes_transferred
   );

   void read_command(std::shared_ptr<Session> session);

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
    asio::ip::tcp::socket socket
   );

   void start_accept();

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
    asio::io_context &io_context,
    uint16_t port,
    std::chrono::milliseconds lock_timeout,
    std::ostream *log_pointer
   );

   uint16_t get_port() const {return port;}
   bool is_readonly() const {return client.is_readonly() || !push_client;}
   std::chrono::milliseconds get_time_stamp() const;

   // Note: run on io_context if on another thread: io_context.post([&](){server.stop();});
   void start();
   void stop_after_sessions();
   void stop();

   ~Server();
 };
}

#endif
