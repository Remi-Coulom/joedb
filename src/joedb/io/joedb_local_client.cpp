#include "joedb/concurrency/Local_Connection.h"
#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/journal/File.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/run_interpreted_client.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (argc != 2)
  {
   std::cerr << "usage: " << argv[0] << " <file_name>\n";
   return 1;
  }
  else
  {
   const char * const file_name = argv[1];

   std::cout << "Connection... ";
   std::cout.flush();
   Local_Connection<File> connection(file_name);
   Interpreted_Client client(connection, connection.get_file());
   std::cout << "OK\n";

   run_interpreted_client(client);
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
