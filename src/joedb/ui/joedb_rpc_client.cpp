#include "joedb/rpc/Client.h"
#include "joedb/concurrency/Local_Channel.h"
#include "joedb/ui/main_wrapper.h"
#include "../../doc/source/tutorial/src/tutorial/Procedures.h"

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

  Memory_File client_file;
  tutorial::Client database_client(client_file);
  Local_Channel channel((std::string(endpoint_path)));

  tutorial::Procedures procedures(database_client);
  rpc::Client rpc_client(channel, procedures);

  {
   tutorial::procedures::city::Memory_Database city;
   city.set_name("Tombouctou");

   {
    city.soft_checkpoint();
    rpc_client.call(0, city);
   }

   {
    city.soft_checkpoint();
    rpc_client.call(2, city);
   }
  }

  return 0;
 }
}

int main(int argc, char **argv)
{
 return joedb::main_wrapper(joedb::rpc_client, argc, argv);
}
