#include "joedb/concurrency/Server_Connection.h"
#include "joedb/ssh/Forward_Channel.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/run_interpreted_client.h"

#include <sstream>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 int main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (argc < 5)
  {
   std::cerr << "usage: " << argv[0] <<
   " <user> <host> <remote_port> <local_file_name>" <<
   " [<ssh_port> [<ssh_log_level>]]\n";
   return 1;
  }
  else
  {
   const char * const user = argv[1];
   const char * const host = argv[2];
   const char * const file_name = argv[4];

   uint16_t joedb_port = 0;
   std::istringstream(argv[3]) >> joedb_port;

   int ssh_port = 22;
   if (argc > 5)
    std::istringstream(argv[5]) >> ssh_port;

   int ssh_log_level = 0;
   if (argc > 6)
    std::istringstream(argv[6]) >> ssh_log_level;

   ssh::Thread_Safe_Session session(user, host, ssh_port, ssh_log_level);
   ssh::Forward_Channel channel(session, "localhost", joedb_port);
   Server_Connection connection(channel);

   run_interpreted_client(connection, file_name);
  }

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb::main, argc, argv);
}
