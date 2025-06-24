#include "joedb/ui/main_wrapper.h"

#include <iostream>

namespace joedb::rpc
{
 static int server(Arguments &arguments)
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

  return 0;
 }
}

int main(int argc, char **argv)
{
 return joedb::main_wrapper(joedb::rpc::server, argc, argv);
}
