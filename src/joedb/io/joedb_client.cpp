#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/Client_Parser.h"
#include "joedb/io/Client_Command_Processor.h"
#include "joedb/concurrency/Client.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 static int client_main(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  const bool local = true;
  Client_Parser client_parser(local);

  if (argc <= 1)
  {
   std::cerr << "usage: " << argv[0];
   client_parser.print_help(std::cerr);
   return 1;
  }

  Client &client = client_parser.parse(argc - 1, argv + 1);

  Client_Command_Processor processor(client);
  Command_Interpreter interpreter{processor};
  interpreter.set_prompt(true);
  interpreter.set_prompt_string("joedb_client");
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
