#include "joedb/ui/main_wrapper.h"
#include "joedb/rpc/Server.h"

#include <iostream>

namespace joedb
{
 static int rpc_server(Arguments &arguments)
 {
  const std::string_view endpoint_path = arguments.get_string_option
  (
   "socket",
   "endpoint_path",
   "joedb_rpc_server.sock"
  );

  const std::string_view file = arguments.get_next("<file.joedb>");

  if (arguments.missing())
  {
   arguments.print_help(std::cerr);
   return 1;
  }

  std::cout << "file = " << file << '\n';
  std::cout << "endpoint_path = " << endpoint_path << '\n';

  std::vector<std::reference_wrapper<rpc::Procedure>> procedures;
  boost::asio::io_context io_context;

  rpc::Server server(procedures, io_context, std::string(endpoint_path));

  io_context.run();

  return 0;
 }
}

int main(int argc, char **argv)
{
 return joedb::main_wrapper(joedb::rpc_server, argc, argv);
}
