#ifndef joedb_Server_declared
#define joedb_Server_declared

#include "joedb/journal/Writable_Journal.h"

#include <experimental/io_context>
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

  public:
   Server
   (
    joedb::Writable_Journal &journal,
    net::io_context &io_context
   );
 };
}

#endif
