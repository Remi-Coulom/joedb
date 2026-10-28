#ifndef joedb_joedb_server_declared
#define joedb_joedb_server_declared

#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Client.h"
#include "joedb/ui/Parsed_Client.h"
#include "joedb/ui/Arguments.h"
#include "joedb/ui/Parsed_Logger.h"

#include <istream>
#include <ostream>
#include <cstring>
#include <cstdlib>
#include <cmath>

namespace joedb
{
 inline int server
 (
  Arguments &arguments,
  std::istream &in,
  std::ostream &out,
  std::ostream &err
 )
 {
  Parsed_Logger logger(arguments);

  std::string default_endpoint_path = "joedb.sock";
  for (size_t i = 1; i < arguments.size(); i++)
  {
   const beman::cstring_view v = arguments[i];
   if (v.ends_with(".joedb"))
   {
    default_endpoint_path = std::string(v) + ".sock";
    break;
   }
  }

  const std::string_view endpoint_path = arguments.get_string_option
  (
   "socket",
   "endpoint_path",
   default_endpoint_path.c_str()
  );

  const float timeout_seconds = arguments.get_option<float>
  (
   "timeout",
   "seconds",
   0.0f
  );

  const Open_Mode default_open_mode = Open_Mode::write_existing_or_create_new;

  Parsed_Client parsed_client
  (
   logger.get(),
   default_open_mode,
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

  logger.get().log
  (
   "creating server, endpoint_path = " + std::string(endpoint_path) +
   "; timeout = " + std::to_string(timeout_seconds)
  );

  const int thread_count = 1;

  Server server
  (
   logger.get(),
   logger.get_log_level(),
   thread_count,
   std::string(endpoint_path),
   client,
   std::chrono::milliseconds(std::lround(timeout_seconds * 1000))
  );

  server.run();

  return 0;
 }
}

#endif
