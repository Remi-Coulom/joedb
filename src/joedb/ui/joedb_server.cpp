#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/IO_Context_Wrapper.h"
#include "joedb/ui/Client_Parser.h"
#include "joedb/ui/main_exception_catcher.h"

#include <iostream>
#include <cstring>
#include <cstdlib>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 static int server(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  const bool local = true;
  const Open_Mode default_open_mode = Open_Mode::write_existing_or_create_new;

  Client_Parser client_parser(local, default_open_mode);

  if (argc <= 1)
  {
   std::cerr << "usage: " << argv[0];
   std::cerr << " [--port p] [--timeout t] [--share]";
   client_parser.print_help(std::cerr);
   std::cerr << R"RRR(
The timeout is the time (in seconds) during which a client lock is kept.
0 (the default) means there is no timeout, and the lock is kept until the
client unlocks or is disconnected. A client that timed out is not disconnected,
and can still push data: the push will succeed only if there is no conflict.
)RRR";
   return 1;
  }

  int32_t index = 1;

  uint16_t port = 0;
  if (index + 1 < argc && std::strcmp(argv[index], "--port") == 0)
  {
   port = uint16_t(std::atoi(argv[index + 1]));
   index += 2;
  }

  uint32_t timeout = 0;
  if (index + 1 < argc && std::strcmp(argv[index], "--timeout") == 0)
  {
   timeout = uint32_t(std::atoi(argv[index + 1]));
   index += 2;
  }

  bool share = false;
  if (index < argc && std::strcmp(argv[index], "--share") == 0)
  {
   share = true;
   index += 1;
  }

  Pullonly_Client &client = client_parser.parse(argc - index, argv + index);

  IO_Context_Wrapper io_context_wrapper;

  std::cout << "Creating server (port = " << port;
  std::cout << "; timeout = " << timeout;
  std::cout << "; share = " << share << ")\n";

  Server server
  (
   client,
   share,
   io_context_wrapper.io_context,
   port,
   std::chrono::seconds(timeout),
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
 return joedb::main_exception_catcher(joedb::server, argc, argv);
}
