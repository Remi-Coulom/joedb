#include "joedb/ui/main_wrapper.h"
#include "joedb/error/Stream_Logger.h"
#include "joedb/journal/File.h"
#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Writable_Journal_Client.h"

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
    Logger &logger,
    int log_level,
    const std::string &file_name,
    const std::string &endpoint_path,
    std::chrono::milliseconds timeout
   ):
    file(file_name, Open_Mode::write_existing_or_create_new),
    client(file),
    server
    (
     logger,
     log_level,
     1,
     endpoint_path,
     client,
     timeout
    )
   {
   }

   void join() {server.join();}
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

  const int log_level = arguments.get_option<int>("log_level", "level", 100);

  arguments.add_parameter("<file.joedb>+");

  if (arguments.get_remaining_count() == 0)
  {
   arguments.print_help(std::cerr);
   return 1;
  }

  std::list<std::unique_ptr<Server_Data>> servers;

  Stream_Logger logger(std::cerr);

  while (arguments.get_remaining_count())
  {
   const std::string file_name(arguments.get_next());
   std::cerr << "Creating server for: " << file_name << '\n';
   servers.emplace_back
   (
    new Server_Data
    (
     logger,
     log_level,
     file_name,
     file_name + ".sock",
     std::chrono::milliseconds(std::lround(timeout_seconds * 1000))
    )
   );
  }

  for (auto &server: servers)
   server->join();

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_wrapper(joedb::multi_server, argc, argv);
}
