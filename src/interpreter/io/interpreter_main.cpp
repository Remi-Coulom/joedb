#include "Interpreter.h"
#include "Database.h"
#include "File.h"
#include "JournalFile.h"

#include <iostream>

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
 if (argc <= 1)
 {
  Database db;
  Interpreter interpreter(db);
  interpreter.main_loop(std::cin, std::cout);
 }
 else
 {
  File file;

  if (!file.open(argv[1], File::mode_t::write_existing))
  {
   std::cout << "Could not open " << argv[1] << " for writing.\n";
   if (!file.open(argv[1], File::mode_t::read_existing))
   {
    std::cout << "Could not open " << argv[1] << " for reading.\n";
    if (!file.open(argv[1], File::mode_t::create_new))
    {
     std::cout << "Could not create " << argv[1] << ".\n";
     return 1;
    }
   }
  }

  JournalFile journal(file);
  Database db;
  journal.replay_log(db);

  if (journal.get_state() != JournalFile::state_t::no_error)
  {
   std::cout << "JournalFile error: " << int(journal.get_state()) << '\n';
   return 1;
  }

  db.set_listener(journal);
  Interpreter interpreter(db);
  interpreter.main_loop(std::cin, std::cout);
 }

 return 0;
}
