#include "joedb/Multiplexer.h"
#include "joedb/io/Interpreter.h"
#include "joedb/io/main_exception_catcher.h"
#include "joedb/io/File_Parser.h"
#include "joedb/interpreter/Database.h"
#include "joedb/journal/Writable_Journal.h"

#include <iostream>
#include <memory>
#include <sstream>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 static int main(int argc, char **argv)
 /////////////////////////////////////////////////////////////////////////////
 {
  File_Parser file_parser;

  if (argc <= 1)
  {
   std::cerr << "usage: " << argv[0] << " <file> [<blob_file>]\n\n";
   file_parser.print_help(std::cerr);
   return 1;
  }

  int arg_index = 1;

  std::ostream null_stream(nullptr);
  Generic_File &file = file_parser.parse(null_stream, argc, argv, arg_index);

  std::unique_ptr<File_Parser> blob_file_parser;
  Generic_File *blob_file = nullptr;
  std::unique_ptr<Writable_Journal> blob_journal;

  if (arg_index < argc)
  {
   blob_file_parser.reset(new File_Parser());
   blob_file = &blob_file_parser->parse(null_stream, argc, argv, arg_index);

   if (blob_file->get_mode() != Open_Mode::read_existing)
   {
    blob_journal.reset(new Writable_Journal(*blob_file));
    blob_journal->append();
   }
  }

  Database db;

  if (file.get_mode() == Open_Mode::read_existing)
  {
   Readonly_Journal journal(file);
   journal.replay_log(db);
   Readable_Interpreter interpreter
   (
    db,
    blob_file ? blob_file : &file
   );
   interpreter.main_loop(std::cin, std::cout);
  }
  else
  {
   Writable_Journal journal(file);
   journal.lock_pull();
   journal.replay_log(db);
   Multiplexer multiplexer{db, journal};
   Interpreter interpreter
   (
    db,
    multiplexer,
    blob_file ? blob_file : &file,
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
