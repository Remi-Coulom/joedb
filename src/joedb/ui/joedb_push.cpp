#include "joedb/ui/main_exception_catcher.h"
#include "joedb/ui/Client_Parser.h"
#include "joedb/ui/Arguments.h"
#include "joedb/concurrency/Readonly_Client.h"
#include "joedb/Signal.h"

#include <iostream>
#include <limits>
#include <thread>
#include <chrono>
#include <cstring>
#include <cstdlib>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 static int push(Arguments &arguments)
 ////////////////////////////////////////////////////////////////////////////
 {
  const bool follow = arguments.has_option("follow");
  const int64_t until_checkpoint = arguments.get_option<int64_t>
  (
   "until",
   "checkpoint",
   std::numeric_limits<int64_t>::max()
  );

  Client_Parser client_parser
  (
   Open_Mode::read_existing,
   Client_Parser::DB_Type::none,
   arguments
  );

  if (!client_parser.get())
  {
   arguments.print_help(std::cerr) << '\n';
   client_parser.print_help(std::cerr);
   return 1;
  }

  Client &client = *client_parser.get();
  Readonly_Client *readonly_client = dynamic_cast<Readonly_Client*>(&client);
  JOEDB_RELEASE_ASSERT(readonly_client);

  if (follow)
  {
   Signal::start();

   while
   (
    client.get_connection_checkpoint() < until_checkpoint &&
    Signal::get_signal() != SIGINT
   )
   {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    client.push_if_ahead();
   }
  }

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb::push, argc, argv);
}
