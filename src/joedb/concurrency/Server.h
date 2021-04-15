#ifndef joedb_Server_declared
#define joedb_Server_declared

#include "joedb/journal/Writable_Journal.h"

#include <list>
#include <queue>
#include <atomic>

#include <experimental/io_context>
#include <experimental/internet>

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

   struct Connection_Data
   {
    net::ip::tcp::socket socket;
    enum {buffer_size = 1024};
    char buffer[buffer_size];
    bool locking;

    Connection_Data(net::ip::tcp::socket &&socket):
     socket(std::move(socket)),
     locking(false)
    {
    }
   };

   std::list<Connection_Data> connections;
   typedef std::list<Connection_Data>::iterator Connection;

   bool locked;
   std::queue<Connection> lock_queue;
   void lock_dequeue();

   void read_handler
   (
    Connection connection,
    const std::error_code &error,
    size_t bytes_transferred
   );

   void read_some(Connection connection);

   void write_handler
   (
    Connection connection,
    const std::error_code &error,
    size_t bytes_transferred
   );

   void write_buffer(Connection connection, size_t size);

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
