#include "Interpreter.h"
#include "Database.h"
#include "File.h"
#include "Journal_File.h"
#include "Readable_Multiplexer.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 if (argc <= 1)
 {
  joedb::Database db;
  joedb::Interpreter interpreter(db);
  interpreter.main_loop(std::cin, std::cout);
 }
 else
 {
  joedb::File file(argv[1], joedb::File::mode_t::automatic);
  joedb::Journal_File journal(file);
  joedb::Database db;
  journal.replay_log(db);
  joedb::Readable_Multiplexer multiplexer(db);
  multiplexer.add_writeable(journal);
  joedb::Interpreter interpreter(multiplexer);
  interpreter.main_loop(std::cin, std::cout);
 }

 return 0;
}
