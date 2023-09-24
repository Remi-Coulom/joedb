#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/Connection_Parser.h"
#include "joedb/io/Client_Command_Processor.h"
#include "joedb/io/Connection_Builder.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/Journal_Client_Data.h"
#include "joedb/concurrency/Interpreted_Client_Data.h"

#include <cstring>
#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 static int client_main(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  Connection_Parser connection_parser(true, true);

  if (argc <= 1)
  {
   std::cerr << "usage: " << argv[0];
   std::cerr << " [--shared|--readonly] [--nodb] <client_file_name> <connection>\n";
   connection_parser.list_builders(std::cerr);
   return 1;
  }

  int arg_index = 1;

  joedb::Open_Mode open_mode = Open_Mode::write_existing_or_create_new;
  if (arg_index < argc)
  {
   if (std::strcmp(argv[arg_index], "--shared") == 0)
   {
    open_mode = Open_Mode::shared_write;
    arg_index++;
   }
   else if (std::strcmp(argv[arg_index], "--readonly") == 0)
   {
    open_mode = Open_Mode::read_existing;
    arg_index++;
   }
  }

  bool nodb = false;
  if (arg_index < argc && std::strcmp(argv[arg_index], "--nodb") == 0)
  {
   nodb = true;
   arg_index++;
  }

  const char *file_name = nullptr;
  if (arg_index < argc)
  {
   file_name = argv[arg_index];
   arg_index++;
  }

  std::cout << "Creating client data... ";
  std::cout.flush();

  std::unique_ptr<Generic_File> client_file;

  if (file_name && *file_name)
   client_file.reset(new File(file_name, open_mode));
  else
   client_file.reset(new Memory_File());

  std::unique_ptr<Client_Data> client_data
  (
   nodb ?
   (Client_Data *)new Journal_Client_Data(*client_file) :
   (Client_Data *)new Interpreted_Client_Data(*client_file)
  );

  std::cout << "OK\n";

  std::cout << "Creating connection... ";
  std::cout.flush();

  std::unique_ptr<Connection> connection = connection_parser.build
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
   interpreter.set_prompt_string("joedb_client(db)");

  interpreter.main_loop(std::cin, std::cout);

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::main_exception_catcher(joedb::client_main, argc, argv);
}
