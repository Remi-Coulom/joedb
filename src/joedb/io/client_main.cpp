#include "joedb/io/client_main.h"
#include "joedb/io/Connection_Builder.h"
#include "joedb/io/Client_Command_Processor.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/concurrency/Journal_Client_Data.h"
#include "joedb/concurrency/Interpreted_Client_Data.h"
#include "joedb/concurrency/Client.h"

#include <cstring>
#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 int client_main(int argc, char **argv, Connection_Builder &&builder)
 ////////////////////////////////////////////////////////////////////////////
 {
  int arg_index = 1;
  bool nodb = false;
  bool shared = builder.get_default_sharing();

  if (builder.has_sharing_option())
  {
   if (arg_index < argc && std::strcmp(argv[arg_index], "--shared") == 0)
   {
    shared = true;
    arg_index++;
   }
  }

  if (arg_index < argc && std::strcmp(argv[arg_index], "--nodb") == 0)
  {
   nodb = true;
   arg_index++;
  }

  const int parameters = argc - arg_index - 1;

  if
  (
   parameters < builder.get_min_parameters() ||
   parameters > builder.get_max_parameters()
  )
  {
   std::cerr << "usage: " << argv[0];
   if (builder.has_sharing_option())
    std::cerr << " [--shared]";
   std::cerr << " [--nodb] <client_file_name> ";
   std::cerr << builder.get_parameters_description() << '\n';
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

   std::unique_ptr<Connection> connection = builder.build
   (
    argc - arg_index,
    argv + arg_index
   );

   std::cout << "OK\n";

   Client client(*client_data, *connection);

   const int64_t diff = client.get_checkpoint_difference();
 
   if (diff > 0)
    std::cout << "You can push " << diff << " bytes.\n";
   else if (diff < 0)
    std::cout << "You can pull " << -diff << " bytes.\n";
   else
    std::cout << "Client data is in sync with the connection.\n";
 
   Client_Command_Processor processor(client);
   Command_Interpreter interpreter{processor};
   interpreter.set_prompt(true);
 
   if (nodb)
    interpreter.set_prompt_string("joedb_client(nodb)");
   else
    interpreter.set_prompt_string("joedb_client");
 
   interpreter.main_loop(std::cin, std::cout);
  }

  return 0;
 }
}
