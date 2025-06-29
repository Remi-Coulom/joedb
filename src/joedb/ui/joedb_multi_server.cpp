#include "joedb/ui/main_wrapper.h"
#include "joedb/journal/File.h"
#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Writable_Journal_Client.h"
#include "joedb/asio/io_context.h"

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
   Writable_Journal_Client client;
   Server server;

  public:
   Server_Data
   (
    boost::asio::io_context &io_context,
    const std::string &file_name,
    const std::string &endpoint_path,
    std::chrono::milliseconds timeout
   ):
    file(file_name, Open_Mode::write_existing_or_create_new),
    client(file),
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
 static int multi_server(Arguments &arguments)
 ////////////////////////////////////////////////////////////////////////////
 {
  const float timeout_seconds = arguments.get_option<float>
  (
   "timeout",
   "seconds",
   0.0f
  );

  arguments.add_parameter("<file.joedb>+");

  if (arguments.get_remaining_count() == 0)
  {
   arguments.print_help(std::cerr);
   return 1;
  }

  joedb::asio::io_context io_context(1);

  std::list<std::unique_ptr<Server_Data>> servers;

  while (arguments.get_remaining_count())
  {
   const std::string file_name(arguments.get_next());
   std::cerr << "Creating server for: " << file_name << '\n';
   servers.emplace_back
   (
    new Server_Data
    (
     *io_context,
     file_name,
     file_name + ".sock",
     std::chrono::milliseconds(int(timeout_seconds * 1000))
    )
   );
  }

  io_context.run();

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_wrapper(joedb::multi_server, argc, argv);
}
