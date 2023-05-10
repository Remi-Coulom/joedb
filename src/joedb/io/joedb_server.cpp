#include "joedb/concurrency/Server.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/File.h"
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
  if (argc != 2 && argc != 4 && argc != 6)
  {
   std::cerr << "usage: " << argv[0];
   std::cerr << " [--port p] [--timeout t] <filename.joedb>\n";
   std::cerr << R"RRR(
The timeout is the time (in seconds) during which a client lock is kept.
0 (the default) means there is no timeout, and the lock is kept until the
client unlocks or is disconnected. A client that timed out is not disconnected,
and can still push data: the push will succeed only if there is no conflict.
)RRR";
   return 1;
  }

  uint16_t port = 0;
  uint32_t timeout = 0;

  int32_t index = 1;

  if (argc >= index + 3 && std::strcmp(argv[index], "--port") == 0)
  {
   port = uint16_t(std::atoi(argv[index + 1]));
   index += 2;
  }

  if (argc >= index + 3 && std::strcmp(argv[index], "--timeout") == 0)
  {
   timeout = uint32_t(std::atoi(argv[index + 1]));
   index += 2;
  }

  File file
  (
   argv[argc - 1],
   Open_Mode::write_existing_or_create_new
  );

  Writable_Journal journal(file);

  net::io_context io_context;
  Server server(journal, io_context, port, timeout, &std::cerr, nullptr);
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
