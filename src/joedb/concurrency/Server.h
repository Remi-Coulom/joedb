#ifndef joedb_Server_declared
#define joedb_Server_declared

#include "joedb/journal/Buffer.h"
#include "joedb/concurrency/Writable_Journal_Client.h"
#include "joedb/ui/Progress_Bar.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/signal_set.hpp>

#include <queue>
#include <iosfwd>
#include <set>
#include <chrono>
#include <optional>
#include <map>

namespace joedb
{
 /// @ingroup concurrency
 class Server
 {
  private:
   boost::asio::io_context &io_context;
   const std::string endpoint_path;
   boost::asio::local::stream_protocol::endpoint endpoint;
   boost::asio::local::stream_protocol::acceptor acceptor;
   bool stopped;
   boost::asio::signal_set interrupt_signals;

   Client &client;
   Writable_Journal_Client *writable_journal_client;
   std::optional<Writable_Journal_Client_Lock> client_lock;

   int64_t session_id;

   struct Session: public std::enable_shared_from_this<Session>
   {
    const int64_t id;
    Server &server;
    boost::asio::local::stream_protocol::socket socket;
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

    char push_status;
    int64_t push_remaining_size;
    std::optional<Async_Writer> push_writer;
    bool unlock_after_push;

    std::optional<boost::asio::steady_timer> pull_timer;
    bool lock_before_pulling;
    bool send_pull_data;
    int64_t pull_checkpoint;

    std::ostream &write_id(std::ostream &out) const;
    std::optional<Progress_Bar> progress_bar;

    Session(Server &server, boost::asio::local::stream_protocol::socket &&socket);
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

   void start_accept();

   static const std::map<char, const char *> request_description;

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
    Client &client,
    boost::asio::io_context &io_context,
    std::string endpoint_path,
    std::chrono::milliseconds lock_timeout,
    std::ostream *log_pointer
   );

   const std::string &get_endpoint_path() const {return endpoint_path;}
   bool has_client_lock() const {return bool(client_lock);}

   // Note: must be run on thread of io_context:
   // asio::post(io_context->get_executor(), [&](){server.stop();});
   void start();
   void stop_after_sessions();
   void stop();

   ~Server();
 };
}

#endif
