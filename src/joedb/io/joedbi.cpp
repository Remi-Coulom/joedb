#include "joedb/Multiplexer.h"
#include "joedb/io/Interpreter.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/interpreter/Database.h"
#include "joedb/journal/File.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/journal/Memory_File.h"

#include <iostream>
#include <memory>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  Database db;

  if (argc <= 1)
  {
   Interpreter interpreter(db, db);
   interpreter.main_loop(std::cin, std::cout);
  }
  else
  {
   const char * const file_name = argv[1];

   std::unique_ptr<Generic_File> file;
   std::unique_ptr<Writable_Journal> writable_journal;

   try
   {
    file.reset(new File(file_name, Open_Mode::write_existing_or_create_new));
    writable_journal.reset(new Writable_Journal(*file));
   }
   catch (const Exception &e)
   {
    std::cout << e.what() << '\n';
    std::cout << "Opening file read-only.\n";
    file.reset(new File(file_name, Open_Mode::read_existing));
   }

   db.set_blob_storage(file.get());

   if (file->get_mode() == Open_Mode::read_existing)
   {
    Readonly_Journal journal(*file);
    journal.replay_log(db);
    Readonly_Interpreter interpreter(db);
    interpreter.main_loop(std::cin, std::cout);
   }
   else
   {
    writable_journal->replay_log(db);
    Multiplexer multiplexer{db, *writable_journal};
    Interpreter interpreter(db, multiplexer);
    interpreter.main_loop(std::cin, std::cout);
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
