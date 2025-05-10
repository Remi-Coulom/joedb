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
 static int joedb_push(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  Arguments arguments(argc, argv);
  const bool follow = arguments.has_option("follow");
  const int64_t until_checkpoint = arguments.get_option<int64_t>
  (
   "until",
   "checkpoint",
   std::numeric_limits<int64_t>::max()
  );

  const Open_Mode default_mode = Open_Mode::read_existing;
  Client_Parser client_parser(default_mode, Client_Parser::DB_Type::none);

  if (!arguments.get_remaining_count())
  {
   arguments.print_help(std::cerr);
   client_parser.print_help(std::cerr);
   return 1;
  }

  Client &client = client_parser.parse(argc - arguments.get_index(), argv + arguments.get_index());
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
 return joedb::main_exception_catcher(joedb::joedb_push, argc, argv);
}
