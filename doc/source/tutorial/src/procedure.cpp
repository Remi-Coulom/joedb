#include "tutorial/procedures/get_population.h"
#include "tutorial/File_Client.h"
#include "joedb/ui/main_wrapper.h"

static int procedure(joedb::Arguments &arguments)
{
 const std::string_view city_name = arguments.get_next("<city_name>");

 if (arguments.missing())
 {
  arguments.print_help(std::cerr);
  return 1;
 }

 tutorial::File_Client client("tutorial.joedb");

 tutorial::procedures::get_population::Memory_Database get_population;
 get_population.set_city_name(std::string(city_name));
 tutorial::procedures::execute(client, get_population);
 std::cout << "population = " << get_population.get_population() << '\n';
 std::cout << "city_id = " << get_population.get_city().get_id() << '\n';
 get_population.soft_checkpoint();

 return 0;
}

int main(int argc, char **argv)
{
 return joedb::main_wrapper(procedure, argc, argv);
}
