#include "joedb/concurrency/Server.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/File.h"

#include <iostream>
#include <joedb/journal/Generic_File.h>

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 if (argc != 2)
 {
  std::cerr << "usage: " << argv[0] << " <filename.joedb>\n";
  return 1;
 }

 joedb::File file(argv[1], joedb::Open_Mode::write_existing_or_create_new);
 joedb::Writable_Journal journal(file);

 net::io_context io_context;
 joedb::Server server(journal, io_context);
 io_context.run();

 return 0;
}
