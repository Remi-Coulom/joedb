#include "joedb/io/Interpreter.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/run_interpreted_client.h"
#include "joedb/ssh/Connection.h"
#include "joedb/ssh/Thread_Safe_Session.h"
#include "joedb/concurrency/Shared_Local_File.h"

#include <sstream>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (argc < 4)
  {
   std::cerr << "usage: " << argv[0];
   std::cerr << " <user> <host> <file_name> [<ssh_port> [<ssh_log_level>]]\n";
   return 1;
  }
  else
  {
   const char *user = argv[1];
   const char *host = argv[2];
   const char *file_name = argv[3];

   int port = 22;
   if (argc > 4)
    std::istringstream(argv[4]) >> port;

   int ssh_log_level = 0;
   if (argc > 5)
    std::istringstream(argv[5]) >> ssh_log_level;

   ssh::Thread_Safe_Session session(user, host, port, ssh_log_level);
   ssh::Connection connection(session, file_name, true);

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
