#include "joedb/ui/main_exception_catcher.h"
#include "joedb/ui/Client_Parser.h"
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
  const bool local = false;
  const bool with_database = false;
  const Open_Mode default_mode = Open_Mode::read_existing;
  Client_Parser client_parser(local, default_mode, with_database);

  int arg_index = 1;

  bool follow = false;
  if (arg_index < argc && std::strcmp(argv[arg_index], "--follow") == 0)
  {
   follow = true;
   arg_index++;
  }

  int64_t until_checkpoint = std::numeric_limits<int64_t>::max();
  if (arg_index + 1 < argc && std::strcmp(argv[arg_index], "--until") == 0)
  {
   until_checkpoint = std::atoll(argv[arg_index + 1]);
   arg_index += 2;
  }

  if (arg_index >= argc)
  {
   std::cerr << "usage: " << argv[0];
   std::cerr << " [--follow] [--until <checkpoint>]";
   client_parser.print_help(std::cerr);
   return 1;
  }

  Client &client = client_parser.parse(argc - arg_index, argv + arg_index);
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
    client.pull();
    if (client.get_journal_checkpoint() > client.get_connection_checkpoint())
     readonly_client->push(Unlock_Action::keep_locked);
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
