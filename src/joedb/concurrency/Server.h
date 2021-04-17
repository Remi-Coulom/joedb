#ifndef joedb_Server_declared
#define joedb_Server_declared

#include "joedb/journal/Writable_Journal.h"

#include <queue>
#include <atomic>
#include <memory>

#include <experimental/io_context>
#include <experimental/internet>
#include <experimental/socket>

namespace net = std::experimental::net;

#ifndef CDECL
#define CDECL
#endif

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Server
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   static std::atomic<bool> interrupted;
   static void CDECL signal_handler(int sig);
   enum {interrupt_check_seconds = 2};

   joedb::Writable_Journal &journal;
   net::io_context &io_context;
   net::ip::tcp::acceptor acceptor;
   net::steady_timer timer;

   struct Session
   {
    net::ip::tcp::socket socket;
    enum {buffer_size = (1 << 13)};
    char buffer[buffer_size];
    bool locking;

    Session(net::ip::tcp::socket &&socket);
    ~Session();
   };

   bool locked;
   std::queue<std::shared_ptr<Session>> lock_queue;
   void lock_dequeue();

   void push_transfer_handler
   (
    std::shared_ptr<Session> session,
    int64_t size,
    std::unique_ptr<Writable_Journal::Tail_Writer> writer,
    const std::error_code &error,
    size_t bytes_transferred
   );

   void push_transfer
   (
    std::shared_ptr<Session> session,
    int64_t size,
    std::unique_ptr<Writable_Journal::Tail_Writer> writer
   );

   void push_handler
   (
    std::shared_ptr<Session> session,
    const std::error_code &error,
    size_t bytes_transferred
   );

   void pull_transfer_handler
   (
    std::shared_ptr<Session> session,
    Async_Reader reader,
    const std::error_code &error,
    size_t bytes_transferred
   );

   void pull_handler
   (
    std::shared_ptr<Session> session,
    const std::error_code &error,
    size_t bytes_transferred
   );

   void read_command_handler
   (
    std::shared_ptr<Session> session,
    const std::error_code &error,
    size_t bytes_transferred
   );

   void read_command(std::shared_ptr<Session> session);

   void write_handler
   (
    std::shared_ptr<Session> session,
    const std::error_code &error,
    size_t bytes_transferred
   );

   void write_buffer(std::shared_ptr<Session> session, size_t size);

   void start_accept();
   void handle_accept
   (
    std::error_code error,
    net::ip::tcp::socket socket
   );

   void start_interrupt_timer();
   void handle_interrupt_timer(std::error_code error);

  public:
   Server
   (
    joedb::Writable_Journal &journal,
    net::io_context &io_context,
    uint16_t port
   );
 };
}

#endif
