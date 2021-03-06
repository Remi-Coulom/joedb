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
  if (argc != 4)
  {
   std::cerr << "usage: " << argv[0] << " <host> <port> <file_name>\n";
   return 1;
  }
  else
  {
   const char * const host = argv[1];
   const char * const port = argv[2];
   const char * const file_name = argv[3];

   Network_Channel channel(host, port);
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
