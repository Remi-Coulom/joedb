#include "joedb/concurrency/Server_Connection.h"
#include "joedb/concurrency/Shared_Local_File.h"
#include "joedb/concurrency/Network_Channel.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/run_interpreted_client.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (argc != 3 && argc != 4)
  {
   std::cerr << "usage: " << argv[0];
   std::cerr << " <host> <port> [<local_file_name>]\n";
   return 1;
  }
  else
  {
   const char * const host = argv[1];
   const char * const port = argv[2];
   const char * const file_name = argc > 3 ? argv[3] : nullptr;

   Network_Channel channel(host, port);
   Server_Connection connection(channel, &std::cerr);

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
