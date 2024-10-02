#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/Client_Parser.h"
#include "joedb/io/Client_Command_Processor.h"
#include "joedb/concurrency/Client.h"
#include "joedb/journal/File.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 static int client_main(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  const bool local = true;

  const Open_Mode default_mode = File::lockable
   ? Open_Mode::shared_write
   : Open_Mode::write_existing_or_create_new;

  Client_Parser client_parser(local, default_mode);

  if (argc <= 1)
  {
   std::cerr << "usage: " << argv[0];
   client_parser.print_help(std::cerr);
   return 1;
  }

  Pullonly_Client &client = client_parser.parse(argc - 1, argv + 1);

  Client_Command_Processor interpreter(client);
  interpreter.set_prompt(true);
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
