#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/Connection_Builder.h"
#include "joedb/io/Connection_Parser.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Readonly_Journal.h"

#include <thread>
#include <chrono>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 static int follow(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  Connection_Parser connection_parser;

  if (argc < 2)
  {
   std::cerr << "usage: " << argv[0] << " <file_name> [<connection> ...]\n";
   connection_parser.list_builders();
   return 1;
  }
  else
  {
   const char * const file_name = argv[1];
   File file(file_name, Open_Mode::read_existing);
   Readonly_Journal journal(file);

   auto connection = connection_parser.build(argc - 2, argv + 2);

   if (!connection)
    return 1;
   else
   {
    int64_t server_checkpoint = connection->handshake(journal);

    connection->lock(journal);

    while (true)
    {
     std::this_thread::sleep_for(std::chrono::seconds(1));
     journal.refresh_checkpoint();
     const int64_t new_checkpoint = journal.get_checkpoint_position();

     if (new_checkpoint > server_checkpoint)
     {
      connection->push(journal, server_checkpoint, false);
      server_checkpoint = new_checkpoint;
     }
    }
   }
  }

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_exception_catcher(joedb::follow, argc, argv);
}
