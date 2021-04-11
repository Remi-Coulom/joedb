#include "joedb/concurrency/Server.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/File.h"

#include <iostream>
#include <sstream>
#include <cstring>
#include <joedb/journal/Generic_File.h>

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 if (argc != 2 && argc != 4)
 {
  std::cerr << "usage: " << argv[0] << " [--port p] <filename.joedb>\n";
  return 1;
 }

 uint16_t port = 0;

 if (argc >= 4 && std::strcmp(argv[1], "--port") == 0)
  std::istringstream(argv[2]) >> port;

 joedb::File file(argv[1], joedb::Open_Mode::write_existing_or_create_new);
 joedb::Writable_Journal journal(file);

 net::io_context io_context;
 joedb::Server server(journal, io_context, port);
 io_context.run();

 return 0;
}
