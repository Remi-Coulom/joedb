#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/Connection_Builder.h"
#include "joedb/io/Connection_Parser.h"
#include "joedb/io/File_Parser.h"
#include "joedb/journal/Readonly_Journal.h"
#include "joedb/concurrency/Connection.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  const bool local = false;
  File_Parser file_parser(Open_Mode::read_existing);
  Connection_Parser connection_parser(local);

  int arg_index = 1;
  bool follow = false;

  if (arg_index < argc && std::strcmp(argv[arg_index], "--follow") == 0)
  {
   follow = true;
   arg_index++;
  }

  if (arg_index >= argc)
  {
   std::cerr << "usage: " << argv[0];
   std::cerr << " [--follow] <file> <connection>\n\n";
   file_parser.print_help(std::cerr);
   connection_parser.print_help(std::cerr);
   return 1;
  }

  Generic_File &file = file_parser.parse(std::cout, argc, argv, arg_index);
  Readonly_Journal journal(file);

  std::unique_ptr<Connection> connection = connection_parser.build
  (
   argc - arg_index,
   argv + arg_index
  );

  if (!connection)
   return 1;
  else
  {
   int64_t server_checkpoint = connection->handshake(journal);

   while (true)
   {
    journal.pull();
    const int64_t new_checkpoint = journal.get_checkpoint_position();

    if (new_checkpoint > server_checkpoint)
    {
     connection->push(journal, server_checkpoint, false);
     server_checkpoint = new_checkpoint;
    }

    if (!follow)
     break;

    std::this_thread::sleep_for(std::chrono::seconds(1));
   }
  }

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb::main, argc, argv);
}
