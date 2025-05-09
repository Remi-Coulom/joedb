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
  if (argc < 2)
  {
   std::cerr << "usage: " << argv[0] << " [--timeout t] <filename.joedb>+\n";
   return 1;
  }

  int32_t index = 1;

  std::chrono::seconds timeout{0};
  if (index + 1 < argc && std::strcmp(argv[index], "--timeout") == 0)
  {
   index++;
   timeout = std::chrono::seconds(std::stoi(argv[index]));
   index++;
  }

  IO_Context_Wrapper io_context_wrapper;

  std::list<std::unique_ptr<Server_Data>> servers;

  for (; index < argc; index++)
  {
   std::string file_name(argv[index]);
   std::cerr << "Creating server for: " << file_name << '\n';
   servers.emplace_back
   (
    new Server_Data
    (
     io_context_wrapper.io_context,
     file_name,
     file_name + ".sock",
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
