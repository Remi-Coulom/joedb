#include "joedb/io/Connection_Builder.h"
#include "joedb/io/run_interpreted_client.h"
#include "joedb/concurrency/Journal_Client.h"
#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Memory_File.h"

#include <iostream>
#include <cstring>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Connection_Builder::open_local_file(const char *file_name)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (file_name && *file_name)
   local_file.reset(new File(file_name, Open_Mode::shared_write));
  else
   local_file.reset(new Memory_File());
 }

 ////////////////////////////////////////////////////////////////////////////
 int Connection_Builder::main(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  bool journal = false;
  int arg_index = 1;

  if (argc >= 2 && std::strcmp(argv[1], "--journal") == 0)
  {
   journal = true;
   arg_index++;
  }

  const int parameters = argc - arg_index;

  if
  (
   parameters < get_min_parameters() ||
   parameters > get_max_parameters()
  )
  {
   std::cerr << "usage: " << argv[0] << " [--journal] ";
   std::cerr << get_parameters_description() << '\n';
   return 1;
  }
  else
  {
   std::cout << "Connection... ";
   std::cout.flush();
   build(argc - arg_index, argv + arg_index);
   std::cout << "OK\n";

   if (journal)
   {
    Journal_Client client(get_connection(), get_file());
    run_interpreted_client(client);
   }
   else
   {
    Interpreted_Client client(get_connection(), get_file());
    run_interpreted_client(client);
   }
  }

  return 0;
 }
}
