#include "tutorial/Generic_File_Database.h"
#include "tutorial/Local_Client.h"
#include "tutorial/Readable.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/Interpreter.h"

#include <iostream>
#include <joedb/Multiplexer.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 ////////////////////////////////////////////////////////////////////////////
 {
  tutorial::Local_Client client("tutorial.joedb");

  client.transaction([](tutorial::Generic_File_Database &db){
   tutorial::Readable readable(db);
   joedb::Multiplexer multiplexer = db.get_multiplexer();
   Interpreter interpreter(readable, multiplexer, nullptr, multiplexer, 0);
   interpreter.main_loop(std::cin, std::cout);
  });

  return 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::main_exception_catcher(joedb::main, argc, argv);
}
