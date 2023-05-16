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
  std::unique_ptr<Generic_File> file;
  std::unique_ptr<Writable_Journal> writable_journal;

  if (argc > 1)
  {
   const char * const file_name = argv[1];

   try
   {
    file.reset(new File(file_name, Open_Mode::write_existing_or_create_new));
   }
   catch (const Exception &e)
   {
    std::cout << e.what() << '\n';
    std::cout << "Opening file read-only.\n";
    file.reset(new File(file_name, Open_Mode::read_existing));
   }
  }
  else
   file.reset(new Memory_File());

  Database db;

  std::unique_ptr<Generic_File> blob_file;
  std::unique_ptr<Writable_Journal> blob_journal;

  if (argc > 2)
  {
   const char * const blob_file_name = argv[2];
   blob_file.reset(new File(blob_file_name, file->get_mode()));

   if (file->get_mode() != Open_Mode::read_existing)
   {
    blob_journal.reset(new Writable_Journal(*blob_file));
    blob_journal->append();
   }
  }

  if (file->get_mode() == Open_Mode::read_existing)
  {
   Readonly_Journal journal(*file);
   journal.replay_log(db);
   Readable_Interpreter interpreter
   (
    db,
    blob_file ? blob_file.get() : file.get()
   );
   interpreter.main_loop(std::cin, std::cout);
  }
  else
  {
   Writable_Journal journal(*file);
   journal.replay_log(db);
   Multiplexer multiplexer{db, journal};
   Interpreter interpreter
   (
    db,
    multiplexer,
    blob_file ? blob_file.get() : file.get(),
    blob_file ? blob_journal.get() : &journal,
    0
   );
   interpreter.main_loop(std::cin, std::cout);
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
