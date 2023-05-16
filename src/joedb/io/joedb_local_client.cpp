#include "joedb/concurrency/Local_Connection.h"
#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/concurrency/Journal_Client.h"
#include "joedb/journal/File.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/run_interpreted_client.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  bool journal = false;

  if (argc >= 2 && argv[1] == std::string("--journal"))
  {
   journal = true;
   argc--;
   argv++;
  }

  if (argc != 2)
  {
   std::cerr << "usage: " << argv[0] << " [--journal] <file_name>\n";
   return 1;
  }
  else
  {
   const char * const file_name = argv[1];

   std::cout << "Connection... ";
   std::cout.flush();
   Local_Connection<File> connection(file_name);
   std::cout << "OK\n";

   if (journal)
   {
    Journal_Client client(connection, connection.get_file());
    run_interpreted_client(client);
   }
   else
   {
    Interpreted_Client client(connection, connection.get_file());
    run_interpreted_client(client);
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
