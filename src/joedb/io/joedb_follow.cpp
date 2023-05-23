#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/Interpreter_Dump_Writable.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Readonly_Journal.h"

#include <thread>
#include <chrono>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (argc != 2)
  {
   std::cerr << "usage: " << argv[0] << " <file_name>\n";
   return 1;
  }
  else
  {
   const char * const file_name = argv[1];
   File file(file_name, Open_Mode::read_existing);
   Readonly_Journal journal(file);

   Interpreter_Dump_Writable dump(std::cout);
   dump.set_muted(true);
   journal.replay_log(dump);
   dump.set_muted(false);

   while (true)
   {
    journal.refresh_checkpoint();
    journal.play_until_checkpoint(dump);
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
