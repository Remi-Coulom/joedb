#include "tutorial/procedures/get_population.h"
#include "tutorial/procedures/get_population/Readable.h"
#include "tutorial/File_Client.h"
#include "joedb/ui/main_wrapper.h"
#include "joedb/ui/Readable_Command_Processor.h"

static int procedure(joedb::Arguments &arguments)
{
 arguments.add_parameter("<city_name>*");

 if (arguments.size() < 2)
 {
  arguments.print_help(std::cerr);
  return 1;
 }

 tutorial::File_Client client("tutorial.joedb");

 tutorial::procedures::get_population::Memory_Database get_population;

 for (size_t i = 1; i < arguments.size(); i++)
 {
  get_population.set_city_name
  (
   get_population.new_data(),
   std::string(arguments[i])
  );
 }

 tutorial::procedures::execute(client, get_population);

 tutorial::procedures::get_population::Readable readable(get_population);
 joedb::Readable_Command_Processor processor(readable);
 processor.print_table
 (
  std::cout,
  tutorial::procedures::get_population::interpreted_data::table_id
 );

 get_population.soft_checkpoint();

 return 0;
}

int main(int argc, char **argv)
{
 return joedb::main_wrapper(procedure, argc, argv);
}
