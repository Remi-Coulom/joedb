#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Client.h"
#include "joedb/ui/Client_Parser.h"
#include "joedb/ui/main_wrapper.h"
#include "joedb/ui/Arguments.h"
#include "joedb/ui/Parsed_Logger.h"

#include <iostream>
#include <cstring>
#include <cstdlib>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 static int server(Arguments &arguments)
 /////////////////////////////////////////////////////////////////////////////
 {
  Parsed_Logger logger(arguments);

  std::string default_endpoint_path = "joedb.sock";
  for (size_t i = 1; i < arguments.size(); i++)
  {
   const std::string_view v = arguments[i];
   if (v.size() > 6 && v.compare(v.size() - 6, 6, ".joedb") == 0)
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

  Client_Parser client_parser
  (
   logger.get(),
   default_open_mode,
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

  server.join();

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_wrapper(joedb::server, argc, argv);
}
