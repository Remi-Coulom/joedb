#include "joedb/concurrency/Server_Connection.h"
#include "joedb/ssh/Forward_Channel.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/run_interpreted_client.h"

#include <cstdlib>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (argc < 4)
  {
   std::cerr << "usage: " << argv[0] <<
   " <user> <host> <joedb_port> [<local_file_name>" <<
   " [<ssh_port> [<ssh_log_level>]]]\n";
   return 1;
  }
  else
  {
   const char * const user = argv[1];
   const char * const host = argv[2];
   const uint16_t joedb_port = uint16_t(std::atoi(argv[3]));
   const char * const local_file_name = argc > 4 ? argv[4] : nullptr;
   const int ssh_port = argc > 5 ? std::atoi(argv[5]) : 22;
   const int ssh_log_level = argc > 6 ? std::atoi(argv[6]) : 0;

   ssh::Thread_Safe_Session session(user, host, ssh_port, ssh_log_level);
   ssh::Forward_Channel channel(session, "localhost", joedb_port);
   Server_Connection connection(channel, &std::cerr);

   run_interpreted_client(connection, local_file_name);
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
