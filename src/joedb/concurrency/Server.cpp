#include "joedb/concurrency/Server.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Server::Server
 ////////////////////////////////////////////////////////////////////////////
 (
  joedb::Writable_Journal &journal,
  net::io_context &io_context
 ):
  journal(journal),
  io_context(io_context)
 {
 }
}
