#include "joedb/ui/main_exception_catcher.h"
#include "joedb/journal/File.h"
#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Writable_Journal_Client.h"
#include "joedb/concurrency/IO_Context_Wrapper.h"

#include <iostream>
#include <list>
#include <memory>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Server_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   File file;
   Connection connection;
   Writable_Journal_Client client;
   Server server;

  public:
   Server_Data
   (
    asio::io_context &io_context,
    const std::string &file_name,
    const std::string &endpoint_path,
    std::chrono::seconds timeout
   ):
    file(file_name, Open_Mode::write_existing_or_create_new),
    client(file, connection),
    server
    (
     client,
     io_context,
     endpoint_path,
     timeout,
     &std::cerr
    )
   {
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (argc < 3)
  {
   std::cerr << "usage: " << argv[0] << " <timeout_seconds> <db>+\n";
   std::cerr << "file name: <db>.joedb\n";
   std::cerr << "socket name: <db>.sock\n";
   std::cerr << "example: " << argv[0] << " 10 db1 db2 db3 db4\n";
   return 1;
  }

  std::chrono::seconds timeout(std::stoi(argv[1]));

  IO_Context_Wrapper io_context_wrapper;

  std::list<std::unique_ptr<Server_Data>> servers;

  for (int i = 2; i < argc; i++)
  {
   std::string base_name(argv[i]);
   servers.emplace_back
   (
    new Server_Data
    (
     io_context_wrapper.io_context,
     base_name + ".joedb",
     base_name + ".sock",
     timeout
    )
   );
  }

  io_context_wrapper.run();

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb::main, argc, argv);
}
