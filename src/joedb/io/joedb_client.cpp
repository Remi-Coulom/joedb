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
  Client_Parser client_parser(true, true);

  if (argc <= 1)
  {
   std::cerr << "usage: " << argv[0];
   client_parser.print_help(std::cerr);
   return 1;
  }

  Client &client = client_parser.parse(argc, argv);

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
