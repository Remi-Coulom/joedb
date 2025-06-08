#include "tutorial.procedures/Get_Population.h"
#include "tutorial/procedures/population/Client.h"
#include "tutorial/procedures/population/Readable.h"
#include "tutorial/File_Client.h"
#include "joedb/ui/main_wrapper.h"
#include "joedb/ui/Readable_Command_Processor.h"
#include "joedb/journal/File_View.h"

namespace population = tutorial::procedures::population;

static int procedure(joedb::Arguments &arguments)
{
 arguments.add_parameter("<city_name>*");

 if (arguments.size() == 1)
 {
  arguments.print_help(std::cerr);
  return 1;
 }

 tutorial::File_Client tutorial_client("tutorial.joedb");
 tutorial::procedures::Get_Population get_population(tutorial_client);

 joedb::Memory_File file;
 population::Client client(file);

 client.transaction([&arguments](population::Writable_Database &population)
 {
  for (size_t i = 1; i < arguments.size(); i++)
  {
   const auto data = population.new_data();
   population.set_city_name(data, std::string(arguments[i]));
  }
 });

 {
  joedb::File_View file_view(file);
  ((joedb::rpc::Procedure *)&get_population)->execute(file_view);
 }

 client.pull();

 {
  const auto &population = client.get_database();

  population::Readable readable(population);
  joedb::Readable_Command_Processor processor(readable);
  processor.print_table
  (
   std::cout,
   population::data_table::id
  );
 }

 return 0;
}

int main(int argc, char **argv)
{
 return joedb::main_wrapper(procedure, argc, argv);
}
