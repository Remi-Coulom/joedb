#include "joedb/io/Connection_Builder.h"
#include "joedb/io/run_client_interpreter.h"
#include "joedb/concurrency/Journal_Client.h"
#include "joedb/concurrency/Interpreted_Client.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/journal/Writable_Journal.h"

#include <iostream>
#include <cstring>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Connection_Builder::open_client_file(const char *file_name)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (file_name && *file_name)
   client_file.reset
   (
    new File
    (
     file_name,
     shared ?
     Open_Mode::shared_write :
     Open_Mode::write_existing_or_create_new
    )
   );
  else
   client_file.reset(new Memory_File());

  client_journal.reset(new Writable_Journal(*client_file));
 }

 ////////////////////////////////////////////////////////////////////////////
 int Connection_Builder::main(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  int arg_index = 1;

  if (arg_index < argc && std::strcmp(argv[arg_index], "--nodb") == 0)
  {
   nodb = true;
   arg_index++;
  }

  if (arg_index < argc && std::strcmp(argv[arg_index], "--shared") == 0)
  {
   shared = true;
   arg_index++;
  }

  const int parameters = argc - arg_index;

  if
  (
   parameters < get_min_parameters() ||
   parameters > get_max_parameters()
  )
  {
   std::cerr << "usage: " << argv[0] << " [--nodb] [--shared] ";
   std::cerr << get_parameters_description() << '\n';
   return 1;
  }
  else
  {
   std::cout << "Connection... ";
   std::cout.flush();
   build(argc - arg_index, argv + arg_index);
   std::cout << "OK\n";

   std::cout << "Creating client... ";
   std::cout.flush();

   std::unique_ptr<Client> client
   (
    nodb ?
    (Client *)new Journal_Client(get_connection()):
    (Client *)new Interpreted_Client(get_connection())
   );

   std::cout << "OK\n";
   run_client_interpreter(*client);
  }

  return 0;
 }
}
