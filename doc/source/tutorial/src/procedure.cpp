#include "tutorial.procedures/get_population.h"
#include "tutorial.procedures/insert_city.h"
#include "tutorial.procedures/delete_city.h"
#include "tutorial/procedures/population/print_table.h"
#include "tutorial/File_Client.h"
#include "joedb/ui/main_wrapper.h"

/////////////////////////////////////////////////////////////////////////////
static int procedure(joedb::Arguments &arguments)
/////////////////////////////////////////////////////////////////////////////
{
 //
 // Parse command-line arguments
 //
 arguments.add_parameter("<city_name>*");

 if (arguments.size() == 1)
 {
  arguments.print_help(std::cerr);
  return 1;
 }

 //
 // Procedure Setup (RPC server side)
 //
 tutorial::File_Client client("tutorial.joedb");

 tutorial::procedures::population::Read_Procedure get_population
 (
  client.get_database(),
  tutorial::procedures::get_population
 );

 tutorial::procedures::city::Write_Procedure insert_city
 (
  client,
  tutorial::procedures::insert_city
 );

 tutorial::procedures::city::Write_Procedure delete_city
 (
  client,
  tutorial::procedures::delete_city
 );

 {
  tutorial::procedures::population::Memory_Database population;

  for (size_t i = 1; i < arguments.size(); i++)
   population.set_city_name(population.new_data(), std::string(arguments[i]));

  get_population.execute_locally(population);
  tutorial::procedures::population::print_data_table(std::cout, population);
 }

 {
  tutorial::procedures::city::Memory_Database city;
  city.set_name("Tombouctou");
  insert_city.execute_locally(city);
  delete_city.execute_locally(city);
 }

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_wrapper(procedure, argc, argv);
}
