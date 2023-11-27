#include "joedb/concurrency/Server.h"
#include "joedb/concurrency/Client.h"
#include "joedb/io/Client_Parser.h"
#include "joedb/io/main_exception_catcher.h"

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
  Client_Parser client_parser(local);

  if (argc <= 1)
  {
   std::cerr << "usage: " << argv[0];
   std::cerr << " [--port p] [--timeout t]";
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

  Client &client = client_parser.parse(argc - index, argv + index);

  net::io_context io_context;

  Server server
  (
   client,
   io_context,
   port,
   std::chrono::seconds(timeout),
   &std::cerr
  );

  io_context.run();

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb::server, argc, argv);
}
