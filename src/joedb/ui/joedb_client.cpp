#include "joedb/ui/main_exception_catcher.h"
#include "joedb/ui/Client_Parser.h"
#include "joedb/ui/Client_Command_Processor.h"
#include "joedb/concurrency/Client.h"
#include "joedb/journal/File.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 static int joedb_client(Arguments &arguments)
 ////////////////////////////////////////////////////////////////////////////
 {
  const Open_Mode default_mode = File::lockable
   ? Open_Mode::shared_write
   : Open_Mode::write_existing_or_create_new;

  Client_Parser client_parser
  (
   default_mode,
   Client_Parser::DB_Type::interpreted,
   arguments
  );

  if (!client_parser.get())
  {
   arguments.print_help(std::cerr) << '\n';
   return 1;
  }

  Client &client = *client_parser.get();

  std::unique_ptr<Client_Command_Processor> interpreter;

  Writable_Client *writable_client = dynamic_cast<Writable_Client*>(&client);

  if (writable_client && !client.is_pullonly())
   interpreter.reset(new Writable_Client_Command_Processor(*writable_client));
  else
   interpreter.reset(new Client_Command_Processor(client));

  interpreter->set_prompt(true);
  interpreter->main_loop(std::cin, std::cout);

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::main_exception_catcher(joedb::joedb_client, argc, argv);
}
