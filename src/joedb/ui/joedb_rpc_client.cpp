#include "joedb/rpc/Client.h"
#include "joedb/concurrency/Local_Channel.h"
#include "joedb/ui/main_wrapper.h"
#include "../../doc/source/tutorial/src/tutorial/Procedures.h"
#include "../../doc/source/tutorial/src/tutorial/procedures/population/print_table.h"

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
  tutorial::Procedures procedures(database_client);

  Local_Channel channel((std::string(endpoint_path)));

  rpc::Client rpc_client(channel, procedures);

  {
   tutorial::procedures::city::Memory_Database city;
   city.set_name("Tombouctou");

   std::cerr << procedures.get_names()[1] << '\n';
   {
    city.soft_checkpoint();
    rpc_client.call(1, city);
   }

   std::cerr << procedures.get_names()[0] << '\n';
   {
    city.soft_checkpoint();
    rpc_client.call(0, city);
   }
  }

  {
   tutorial::procedures::population::Memory_Database population;
   population.set_city_name(population.new_data(), "Paris");
   population.set_city_name(population.new_data(), "Tokyo");

   std::cerr << procedures.get_names()[2] << '\n';
   {
    population.soft_checkpoint();
    rpc_client.call(2, population);
    population.pull();
   }

   tutorial::procedures::population::print_data_table(std::cout, population);
  }

  return 0;
 }
}

int main(int argc, char **argv)
{
 return joedb::main_wrapper(joedb::rpc_client, argc, argv);
}
