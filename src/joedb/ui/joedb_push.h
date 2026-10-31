#ifndef joedb_joedb_push_declared
#define joedb_joedb_push_declared

#include "joedb/ui/Parsed_Client.h"
#include "joedb/ui/Arguments.h"
#include "joedb/ui/Parsed_Logger.h"
#include "joedb/ui/interruptible_sleep.h"
#include "joedb/Signal.h"

#include <istream>
#include <ostream>
#include <limits>
#include <chrono>

namespace joedb
{
 inline int push
 (
  Arguments &arguments,
  std::istream &in,
  std::ostream &out,
  std::ostream &err
 )
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
  client.push_if_ahead(until_checkpoint);

  if (follow)
  {
   Signal::start();

   logger.get().log("joedb_push invoked with --follow: waiting for data");

   while
   (
    client.get_connection_checkpoint() < until_checkpoint &&
    interruptible_sleep(std::chrono::seconds(1))
   )
   {
    client.push_if_ahead(until_checkpoint);
   }
  }

  return 0;
 }
}

#endif
