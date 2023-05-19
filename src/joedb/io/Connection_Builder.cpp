#include "joedb/io/Connection_Builder.h"
#include "joedb/concurrency/Journal_Client_Data.h"
#include "joedb/concurrency/Interpreted_Client_Data.h"
#include "joedb/concurrency/Client.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/io/run_client_interpreter.h"

#include <iostream>
#include <cstring>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 int Connection_Builder::main(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  int arg_index = 1;
  bool nodb = false;
  bool shared = get_default_sharing();

  if (arg_index < argc && std::strcmp(argv[arg_index], "--nodb") == 0)
  {
   nodb = true;
   arg_index++;
  }

  if (has_sharing_option())
  {
   if (arg_index < argc && std::strcmp(argv[arg_index], "--shared") == 0)
   {
    shared = true;
    arg_index++;
   }
  }

  const int parameters = argc - arg_index - 1;

  if
  (
   parameters < get_min_parameters() ||
   parameters > get_max_parameters()
  )
  {
   std::cerr << "usage: " << argv[0];
   if (has_sharing_option())
    std::cerr << " [--shared]";
   std::cerr << " [--nodb] <client_file_name> ";
   std::cerr << get_parameters_description() << '\n';
   return 1;
  }
  else
  {
   std::cout << "Creating client data... ";
   std::cout.flush();

   std::unique_ptr<Generic_File> client_file;

   {
    const char *client_file_name = argv[arg_index++];

    if (*client_file_name)
    {
     client_file.reset
     (
      new File
      (
       client_file_name,
       shared ?
       Open_Mode::shared_write :
       Open_Mode::write_existing_or_create_new
      )
     );
    }
    else
     client_file.reset(new Memory_File());
   }

   std::unique_ptr<Client_Data> client_data
   (
    nodb ?
    (Client_Data *)new Journal_Client_Data(*client_file) :
    (Client_Data *)new Interpreted_Client_Data(*client_file)
   );

   std::cout << "OK\n";

   std::cout << "Creating connection... ";
   std::cout.flush();

   std::unique_ptr<Connection> connection = build
   (
    client_data->get_journal(),
    argc - arg_index,
    argv + arg_index
   );

   std::cout << "OK\n";

   Client client(*client_data, *connection);

   run_client_interpreter(client);
  }

  return 0;
 }
}
