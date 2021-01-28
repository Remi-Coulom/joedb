#include "joedb/io/Interpreter.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/concurrency/SSH_Connection.h"
#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/concurrency/Shared_Local_File.h"
#include "joedb/ssh/Thread_Safe_Session.h"
#include "joedb/journal/File.h"

#include <sstream>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (argc < 5)
  {
   std::cerr << "usage: " << argv[0];
   std::cerr << " <user> <host> <port> <file_name> [<ssh_log_level>]\n";
   return 1;
  }
  else
  {
   const char *user = argv[1];

   const char *host = argv[2];

   int port = 22;
   std::istringstream(argv[3]) >> port;

   const char *file_name = argv[4];

   int ssh_log_level = 0;
   if (argc == 6)
    std::istringstream(argv[5]) >> ssh_log_level;

   ssh::Thread_Safe_Session session(user, host, port, ssh_log_level);
   ssh::Remote_Mutex remote_mutex(session, file_name, true);
   SSH_Connection connection(remote_mutex);

   Shared_Local_File file(file_name);
   Interpreted_Client client(connection, file);

   while (std::cin)
   {
    std::cout << "R(read), P(pull), W(write), or Q(quit)? ";
    std::cout.flush();
    std::string input;

    if (!(std::cin >> input))
    {
     std::cout << '\n';
     break;
    }

    if (input == "W")
    {
     Interpreted_Lock lock(client);
     Interpreter(lock.get_database()).main_loop(std::cin, std::cout);
    }
    else if (input == "P")
     client.pull();
    else if (input == "R")
     Readonly_Interpreter(client.get_database()).main_loop(std::cin, std::cout);
    else if (input == "Q")
     break;
   }
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
