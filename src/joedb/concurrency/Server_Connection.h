#ifndef joedb_Server_Connection_declared
#define joedb_Server_Connection_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/concurrency/Mutex.h"

#include <experimental/io_context>
#include <experimental/internet>
#include <experimental/socket>
#include <experimental/buffer>

namespace net = std::experimental::net;

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Server_Connection: public Connection, public Mutex
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   net::io_context io_context;
   net::ip::tcp::socket socket;

   enum {buffer_size = (1 << 16)};
   char *buffer;

   int64_t pull(Writable_Journal &client_journal) override;

   int64_t lock_pull(Writable_Journal &client_journal) override;

   void push_unlock
   (
    Readonly_Journal &client_journal,
    int64_t server_position
   ) override;

   void lock() override;

   void unlock() override;

  public:
   Server_Connection(const char *host_name, const char *port_name);
   ~Server_Connection() override;
 };
}

#endif
