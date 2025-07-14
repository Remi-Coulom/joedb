#include "joedb/concurrency/Local_Channel.h"
#include "joedb/ui/main_wrapper.h"
#include "tutorial/rpc/Client.h"
#include "tutorial/rpc/population/print_table.h"

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
  tutorial::rpc::Client rpc_client(channel);

  {
   tutorial::rpc::city::Memory_Database city;
   city.set_name("Tombouctou");
   rpc_client.insert_city(city);
   rpc_client.delete_city(city);
  }

  {
   tutorial::rpc::population::Memory_Database population;
   population.set_city_name(population.new_data(), "Tokyo");
   population.set_city_name(population.new_data(), "Tombouctou");
   population.set_city_name(population.new_data(), "Lille");
   rpc_client.get_population(population);
   tutorial::rpc::population::print_data_table(std::cout, population);
  }

  return 0;
 }
}

int main(int argc, char **argv)
{
 return joedb::main_wrapper(joedb::rpc_client, argc, argv);
}
