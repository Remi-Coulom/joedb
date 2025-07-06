#include "joedb/ui/main_wrapper.h"
#include "joedb/rpc/Server.h"
#include "joedb/error/Stream_Logger.h"

#include "../../doc/source/tutorial/src/tutorial/File_Client.h"
#include "../../doc/source/tutorial/src/tutorial/Procedures.h"

#include <iostream>
#include <thread>

namespace joedb
{
 static int rpc_server(Arguments &arguments)
 {
  const std::string_view endpoint_option = arguments.get_string_option
  (
   "socket",
   "endpoint_path",
   ""
  );

  const int log_level = arguments.get_option<int>("log_level", "level", 100);

  const std::string_view file = arguments.get_next("<file.joedb>");

  if (arguments.missing())
  {
   arguments.print_help(std::cerr);
   return 1;
  }

  const std::string endpoint_path = endpoint_option.empty()
   ? std::string(file) + ".rpc.sock"
   : std::string(endpoint_option);

  Stream_Logger logger(std::cerr);

  tutorial::File_Client client(file.data());

  tutorial::Procedures procedures(client);

  rpc::Server server
  (
   logger,
   log_level,
   std::thread::hardware_concurrency(),
   std::string(endpoint_path),
   procedures
  );

  server.join();

  return 0;
 }
}

int main(int argc, char **argv)
{
 return joedb::main_wrapper(joedb::rpc_server, argc, argv);
}
