#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/IO_Context_Wrapper.h"
#include "joedb/ui/Client_Parser.h"
#include "joedb/ui/main_exception_catcher.h"
#include "joedb/ui/Arguments.h"

#include <iostream>
#include <cstring>
#include <cstdlib>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 static int joedb_server(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  std::string default_endpoint_path = "joedb.sock";
  for (int i = 1; i < argc; i++)
  {
   std::string_view v(argv[i]);
   if (v.size() > 6 && v.compare(v.size() - 6, 6, ".joedb") == 0)
   {
    default_endpoint_path = std::string(v) + ".sock";
    break;
   }
  }

  Arguments arguments(argc, argv);

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

  Client_Parser client_parser(default_open_mode, Client_Parser::DB_Type::none);

  if (!arguments.get_remaining_count())
  {
   arguments.print_help(std::cerr);
   client_parser.print_help(std::cerr);
   return 1;
  }

  Client &client = client_parser.parse(arguments);

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
 return joedb::main_exception_catcher(joedb::joedb_server, argc, argv);
}
