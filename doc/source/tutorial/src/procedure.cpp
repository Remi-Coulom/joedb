#include "tutorial.procedures/get_population.h"
#include "tutorial/procedures/population/Memory_Database.h"
#include "tutorial/procedures/population/Readable.h"
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
 // Setup the get_population procedure
 //
 tutorial::File_Client client("tutorial.joedb");
 tutorial::procedures::population::Read_Procedure procedure
 (
  client,
  tutorial::procedures::get_population
 );

 //
 // Set input
 //
 population::Memory_Database db;
 for (size_t i = 1; i < arguments.size(); i++)
  db.set_city_name(db.new_data(), std::string(arguments[i]));

 //
 // This will be executed by the RPC system
 //
 {
  db.Writable_Database::flush();
  joedb::File_View file_view(db.get_file_view());
  ((joedb::rpc::Procedure *)&procedure)->execute(file_view);
  db.pull();
 }

 //
 // Print output
 //
 population::Readable readable(db);
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
