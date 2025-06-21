#include "tutorial/File_Client.h"
#include "tutorial/Procedures.h"
#include "tutorial/procedures/population/print_table.h"
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
 tutorial::Procedures procedures(client);

 //
 // Execute get_population and print result table
 //
 {
  tutorial::procedures::population::Memory_Database population;

  for (size_t i = 1; i < arguments.size(); i++)
   population.set_city_name(population.new_data(), std::string(arguments[i]));

  procedures.get_population.execute_locally(population);
  tutorial::procedures::population::print_data_table(std::cout, population);
 }

 //
 // Also try insert_city and delete_city
 //
 {
  tutorial::procedures::city::Memory_Database city;
  city.set_name("Tombouctou");
  procedures.insert_city.execute_locally(city);
  procedures.delete_city.execute_locally(city);
 }

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_wrapper(procedure, argc, argv);
}
