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
  uint16_t port = 0;
  uint32_t timeout = 0;
  bool readonly = false;
  const char *file_name = nullptr;

  int32_t index = 1;

  if (index + 1 < argc && std::strcmp(argv[index], "--port") == 0)
  {
   port = uint16_t(std::atoi(argv[index + 1]));
   index += 2;
  }

  if (index + 1 < argc && std::strcmp(argv[index], "--timeout") == 0)
  {
   timeout = uint32_t(std::atoi(argv[index + 1]));
   index += 2;
  }

  if (index < argc && std::strcmp(argv[index], "--readonly") == 0)
  {
   readonly = true;
   index += 1;
  }

  if (index < argc)
  {
   file_name = argv[index];
   index += 1;
  }

  if (file_name == nullptr || index != argc)
  {
   std::cerr << "usage: " << argv[0];
   std::cerr << " [--port p] [--timeout t] [--readonly] <filename.joedb>\n";
   std::cerr << R"RRR(
The timeout is the time (in seconds) during which a client lock is kept.
0 (the default) means there is no timeout, and the lock is kept until the
client unlocks or is disconnected. A client that timed out is not disconnected,
and can still push data: the push will succeed only if there is no conflict.
)RRR";
   return 1;
  }

  File file
  (
   file_name,
   readonly ?
   Open_Mode::read_existing :
   Open_Mode::write_existing_or_create_new
  );

  std::unique_ptr<Readonly_Journal> journal;

  if (readonly)
   journal.reset(new Readonly_Journal(file));
  else
   journal.reset(new Writable_Journal(file));

  net::io_context io_context;
  Server server(*journal, io_context, port, timeout, &std::cerr);
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
