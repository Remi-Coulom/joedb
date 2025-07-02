#include "joedb/ui/main_wrapper.h"
#include "joedb/concurrency/Local_Channel.h"

#include <iostream>

namespace joedb
{
 static int rpc_client(Arguments &arguments)
 {
  const std::string_view endpoint_path = arguments.get_next("<endpoint_path>");
  if (arguments.missing())
  {
   arguments.print_help(std::cerr);
   return 1;
  }

  Local_Channel channel((std::string(endpoint_path)));

  while (true)
  {
   std::string data;
   std::cin >> data;
   channel.write(data.data(), data.size());
   channel.read(data.data(), data.size());
   std::cout << data << '\n';
   if (data == "quit")
    break;
  }

  return 0;
 }
}

int main(int argc, char **argv)
{
 return joedb::main_wrapper(joedb::rpc_client, argc, argv);
}
