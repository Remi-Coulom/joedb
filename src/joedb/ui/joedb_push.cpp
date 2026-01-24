#include "joedb/ui/main_wrapper.h"
#include "joedb/ui/Parsed_Client.h"
#include "joedb/ui/Arguments.h"
#include "joedb/ui/Parsed_Logger.h"
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
  Parsed_Logger logger(arguments);

  const bool follow = arguments.has_flag("follow");
  const int64_t until_checkpoint = arguments.get_option<int64_t>
  (
   "until",
   "checkpoint",
   std::numeric_limits<int64_t>::max()
  );

  Parsed_Client parsed_client
  (
   logger.get(),
   Open_Mode::read_existing,
   Parsed_Client::DB_Type::none,
   arguments
  );

  if (!parsed_client.get())
  {
   arguments.print_help(std::cerr) << '\n';
   parsed_client.print_help(std::cerr);
   return 1;
  }

  Client &client = *parsed_client.get();
  client.push_if_ahead(until_checkpoint);

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
    client.push_if_ahead(until_checkpoint);
   }
  }

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_wrapper(joedb::push, argc, argv);
}
