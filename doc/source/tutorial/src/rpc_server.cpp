#include "joedb/ui/main_wrapper.h"
#include "joedb/rpc/Server.h"
#include "joedb/error/CLog_System_Logger.h"

#include "tutorial/File_Client.h"
#include "tutorial/rpc/Procedures.h"
#include "tutorial/rpc/Signatures.h"

#include <iostream>

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

  CLog_System_Logger logger;

  tutorial::File_Client client(file.data());
  tutorial::rpc::Service service(client);

  tutorial::rpc::Procedures procedures(service);

  rpc::Server server
  (
   logger,
   log_level,
   1,
   std::string(endpoint_path),
   tutorial::rpc::get_signatures(),
   procedures.procedures
  );

  server.join();

  return 0;
 }
}

int main(int argc, char **argv)
{
 return joedb::main_wrapper(joedb::rpc_server, argc, argv);
}
