#include "joedb/ui/main_exception_catcher.h"
#include "joedb/ui/Connection_Builder.h"
#include "joedb/ui/Connection_Parser.h"
#include "joedb/ui/File_Parser.h"
#include "joedb/journal/Readonly_Journal.h"
#include "joedb/concurrency/Connection.h"
#include "joedb/Signal.h"

#include <iostream>
#include <limits>
#include <thread>
#include <chrono>
#include <cstring>
#include <cstdlib>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 static int joedb_push(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  const bool local = false;
  File_Parser file_parser(Open_Mode::read_existing, true);
  Connection_Parser connection_parser(local);

  int arg_index = 1;
  bool follow = false;

  if (arg_index < argc && std::strcmp(argv[arg_index], "--follow") == 0)
  {
   follow = true;
   arg_index++;
  }

  int64_t until_checkpoint = std::numeric_limits<int64_t>::max();

  if (arg_index + 1 < argc && std::strcmp(argv[arg_index], "--until") == 0)
  {
   until_checkpoint = std::atoll(argv[arg_index + 1]);
   arg_index += 2;
  }

  if (arg_index >= argc)
  {
   std::cerr << "usage: " << argv[0];
   std::cerr << " [--follow] [--until <checkpoint>] <file> <connection>\n\n";
   file_parser.print_help(std::cerr);
   connection_parser.print_help(std::cerr);
   return 1;
  }

  Buffered_File &file = *file_parser.parse(std::cerr, argc, argv, arg_index);
  Readonly_Journal journal(file);

  Connection &connection = connection_parser.build
  (
   argc - arg_index,
   argv + arg_index,
   &file
  );

  int64_t from_checkpoint = connection.handshake(journal, true);
  Signal::start();

  while
  (
   from_checkpoint < until_checkpoint &&
   Signal::get_signal() != SIGINT
  )
  {
   if (journal.get_checkpoint_position() > from_checkpoint)
   {
    from_checkpoint = connection.push_until
    (
     journal,
     from_checkpoint,
     until_checkpoint,
     false
    );
   }

   if (follow)
   {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    journal.pull();
   }
   else
    break;
  }

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb::joedb_push, argc, argv);
}
