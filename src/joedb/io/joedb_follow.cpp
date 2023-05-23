#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/Interpreter_Dump_Writable.h"
#include "joedb/journal/File.h"
#include "joedb/concurrency/Interpreted_Client_Data.h"
#include "joedb/concurrency/Local_Connection.h"
#include "joedb/concurrency/Client.h"

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
   File file(file_name, Open_Mode::shared_write);
   Interpreter_Dump_Writable dump(std::cout);
   Interpreted_Client_Data data(file, &dump);
   Local_Connection connection;
   dump.set_muted(true);
   joedb::Client client(data, connection);
   dump.set_muted(false);

   while (true)
   {
    client.pull();
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
