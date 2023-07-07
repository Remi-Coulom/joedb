#ifndef joedb_SSH_Server_Connection_declared
#define joedb_SSH_Server_Connection_declared

#include "joedb/ssh/Forward_Channel.h"
#include "joedb/concurrency/Server_Connection.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 class SSH_Server_Connection:
 /////////////////////////////////////////////////////////////////////////////
  private ssh::Session,
  private ssh::Forward_Channel,
  public Server_Connection
 {
  public:
   SSH_Server_Connection
   (
    const char *user,
    const char *host,
    const uint16_t joedb_port,
    const unsigned ssh_port,
    const int ssh_log_level,
    std::ostream *log
   ):
    ssh::Session(user, host, ssh_port, ssh_log_level),
    ssh::Forward_Channel(*this, "localhost", joedb_port),
    Server_Connection(*this, log)
   {
   }
 };
}

#endif
