#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/Shared_Local_File.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/run_interpreted_client.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (argc != 4)
  {
   std::cerr << "usage: " << argv[0] << " <host> <port> <file_name>\n";
   return 1;
  }
  else
  {
   const char *host = argv[1];
   const char *port = argv[2];
   const char *file_name = argv[3];

   Server_Connection connection(host, port);

   Shared_Local_File file(connection, file_name);
   run_interpreted_client(connection, file);
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
