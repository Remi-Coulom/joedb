#include "joedb/ui/main_wrapper.h"
#include "joedb/rpc/Server.h"

#include <iostream>
#include <thread>

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

  rpc::Server server((std::string(endpoint_path)));

  std::vector<std::thread> threads;

  for (int i = int(std::thread::hardware_concurrency()); --i >= 0;)
   threads.emplace_back([&server](){server.get_io_context().run();});

  for (auto &thread: threads)
   thread.join();

  return 0;
 }
}

int main(int argc, char **argv)
{
 return joedb::main_wrapper(joedb::rpc_server, argc, argv);
}
