#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/IO_Context_Wrapper.h"
#include "joedb/ui/Client_Parser.h"
#include "joedb/ui/main_wrapper.h"
#include "joedb/ui/Arguments.h"

#include <iostream>
#include <cstring>
#include <cstdlib>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 static int server(Arguments &arguments)
 /////////////////////////////////////////////////////////////////////////////
 {
  std::string default_endpoint_path = "joedb.sock";
  for (int i = 1; i < arguments.get_argc(); i++)
  {
   const std::string_view v(arguments[i]);
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
   default_endpoint_path
  );

  const float timeout_seconds = arguments.get_option<float>
  (
   "timeout",
   "seconds",
   0.0f
  );

  const Open_Mode default_open_mode = Open_Mode::write_existing_or_create_new;

  Client_Parser client_parser(default_open_mode, Client_Parser::DB_Type::none, arguments);

  if (!client_parser.get())
  {
   arguments.print_help(std::cerr) << '\n';
   client_parser.print_help(std::cerr);
   return 1;
  }

  Client &client = *client_parser.get();

  IO_Context_Wrapper io_context_wrapper;

  std::cout << "Creating server (endpoint_path = " << endpoint_path;
  std::cout << "; timeout = " << timeout_seconds << ")\n";

  Server server
  (
   client,
   io_context_wrapper.io_context,
   std::string(endpoint_path),
   std::chrono::milliseconds(int(timeout_seconds * 1000)),
   &std::cerr
  );

  io_context_wrapper.run();

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_wrapper(joedb::server, argc, argv);
}
