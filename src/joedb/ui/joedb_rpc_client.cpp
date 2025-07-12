#include "joedb/concurrency/Local_Channel.h"
#include "joedb/ui/main_wrapper.h"
#include "joedb/rpc/Client.h"
#include "../../doc/source/tutorial/src/tutorial/rpc/population/print_table.h"
#include "../../doc/source/tutorial/src/tutorial/rpc/Signatures.h"

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

  const auto &signatures = tutorial::rpc::get_signatures();
  rpc::Client rpc_client(channel, tutorial::rpc::get_signatures());

  {
   tutorial::rpc::city::Memory_Database city;
   city.set_name("Tombouctou");

   std::cerr << signatures[0].name << '\n';
   {
    city.soft_checkpoint();
    rpc_client.call(0, city);
   }

   std::cerr << signatures[1].name << '\n';
   {
    city.soft_checkpoint();
    rpc_client.call(1, city);
   }
  }

  {
   tutorial::rpc::population::Memory_Database population;
   population.set_city_name(population.new_data(), "Paris");
   population.set_city_name(population.new_data(), "Tokyo");

   std::cerr << signatures[2].name << '\n';
   {
    population.soft_checkpoint();
    rpc_client.call(2, population);
    population.pull();
   }

   tutorial::rpc::population::print_data_table(std::cout, population);
  }

  return 0;
 }
}

int main(int argc, char **argv)
{
 return joedb::main_wrapper(joedb::rpc_client, argc, argv);
}
