#include "joedb/concurrency/Embedded_Connection.h"
#include "joedb/journal/File.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/run_interpreted_client.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (argc < 2)
  {
   std::cerr << "usage: " << argv[0] << " <server.joedb> [<client.joedb>]\n";
   return 1;
  }
  else
  {
   const char * const server_file_name = argv[1];
   const char * const client_file_name = argc > 2 ? argv[2] : nullptr;

   File server_file(server_file_name, Open_Mode::write_existing_or_create_new);
   Embedded_Connection connection(server_file);
   Interpreted_Client client(connection, server_file);
   run_interpreted_client(connection, client_file_name);
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
