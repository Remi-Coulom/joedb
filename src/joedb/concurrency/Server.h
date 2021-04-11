#ifndef joedb_Server_declared
#define joedb_Server_declared

#include "joedb/journal/Writable_Journal.h"

#include <list>

#include <experimental/io_context>
#include <experimental/internet>

namespace net = std::experimental::net;

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Server
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   joedb::Writable_Journal &journal;
   net::io_context &io_context;
   net::ip::tcp::acceptor acceptor;

   struct Connection_Data
   {
    net::ip::tcp::socket socket;

    Connection_Data(net::ip::tcp::socket &&socket): socket(std::move(socket))
    {
    }
   };

   std::list<Connection_Data> connections;

   void start_accept();
   void handle_accept
   (
    const std::error_code &error,
    net::ip::tcp::socket socket
   );

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
