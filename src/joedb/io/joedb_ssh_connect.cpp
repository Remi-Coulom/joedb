#include "joedb/io/Interpreter.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/concurrency/SSH_Robust_Connection.h"
#include "joedb/concurrency/Interpreted_Client.h"
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
   int port = 22;
   std::istringstream(argv[3]) >> port;

   int ssh_log_level = 0;
   if (argc == 6)
    std::istringstream(argv[5]) >> ssh_log_level;

   SSH_Robust_Connection connection
   (
    argv[1],
    argv[2],
    port,
    argv[4],
    true,
    ssh_log_level
   );

   File file(argv[4], Open_Mode::write_existing_or_create_new);
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
