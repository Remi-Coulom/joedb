#include "joedb/concurrency/Server.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/File.h"
#include "joedb/io/main_exception_catcher.h"

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <joedb/journal/Generic_File.h>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (argc != 2 && argc != 4 && argc != 6)
  {
   std::cerr << "usage: " << argv[0];
   std::cerr << " [--port p] [--timeout t] <filename.joedb>\n";
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
  Server server(journal, io_context, port, timeout);
  io_context.run();

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb::main, argc, argv);
}
