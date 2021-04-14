#ifndef joedb_Server_Connection_declared
#define joedb_Server_Connection_declared

#include "joedb/concurrency/Connection.h"

#include <experimental/io_context>
#include <experimental/internet>

namespace net = std::experimental::net;

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Server_Connection: public Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   net::io_context io_context;
   net::ip::tcp::socket socket;

   int64_t pull(Writable_Journal &client_journal) override;

   int64_t lock_pull(Writable_Journal &client_journal) override;

   void push_unlock
   (
    Readonly_Journal &client_journal,
    int64_t server_position
   ) override;

  public:
   Server_Connection(const char *host_name, const char *port_name);
 };
}

#endif
