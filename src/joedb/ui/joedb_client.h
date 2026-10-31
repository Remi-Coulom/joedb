#ifndef joedb_joedb_client_declared
#define joedb_joedb_client_declared

#include "joedb/ui/Parsed_Client.h"
#include "joedb/ui/Client_Command_Processor.h"
#include "joedb/ui/Parsed_Logger.h"
#include "joedb/concurrency/Client.h"
#include "joedb/journal/File.h"

#include <istream>
#include <ostream>

namespace joedb
{
 inline int client
 (
  Arguments &arguments,
  std::istream &in,
  std::ostream &out,
  std::ostream &err
 )
 {
  Parsed_Logger logger(arguments);

  const Open_Mode default_mode = File::lockable
   ? Open_Mode::shared_write
   : Open_Mode::write_existing_or_create_new;

  Parsed_Client parsed_client
  (
   logger.get(),
   default_mode,
   Parsed_Client::DB_Type::interpreted,
   arguments,
   out
  );

  if (!parsed_client.get())
  {
   arguments.print_help(err) << '\n';
   parsed_client.print_help(err);
   return 1;
  }

  Client &client = *parsed_client.get();

  std::unique_ptr<Client_Command_Processor> interpreter;

  Writable_Client *writable_client = dynamic_cast<Writable_Client*>(&client);

  if (writable_client && !client.is_pullonly())
   interpreter.reset(new Writable_Client_Command_Processor(*writable_client));
  else
   interpreter.reset(new Client_Command_Processor(client));

  interpreter->set_prompt(true);
  interpreter->main_loop(in, out);

  return 0;
 }
}

#endif
