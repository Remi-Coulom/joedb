#include "tutorial.procedures/Get_Population.h"
#include "tutorial/procedures/population/Readable.h"
#include "tutorial/procedures/population/Memory_Database.h"
#include "tutorial/File_Client.h"
#include "joedb/ui/main_wrapper.h"
#include "joedb/ui/Readable_Command_Processor.h"
#include "joedb/journal/File_View.h"

namespace population = tutorial::procedures::population;

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
 // Setup the Get_Population procedure
 //
 tutorial::File_Client tutorial_client("tutorial.joedb");
 tutorial::procedures::Get_Population get_population;
 tutorial::procedures::population::detail::Erased_Procedure procedure
 (
  tutorial_client,
  get_population
 );

 //
 // Set input
 //
 population::Memory_Database population;
 for (size_t i = 1; i < arguments.size(); i++)
  population.set_city_name(population.new_data(), std::string(arguments[i]));

 //
 // This will be executed by the RPC system
 //
 {
  population.Writable_Database::flush();
  joedb::File_View file_view(population.get_file_view());
  procedure.execute(file_view);
  population.pull();
 }

 //
 // Print output
 //
 population::Readable readable(population);
 joedb::Readable_Command_Processor processor(readable);
 processor.print_table
 (
  std::cout,
  population::data_table::id
 );

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_wrapper(procedure, argc, argv);
}
