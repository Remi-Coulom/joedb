#include "joedb/journal/Interpreted_File.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/Multiplexer.h"
#include "joedb/io/Interpreter.h"
#include "joedb/interpreter/Database.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Interpreted_File_Data::Interpreted_File_Data(std::istream &file)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!file)
   throw Exception("bad command stream");

  Database db;
  Writable_Journal journal(memory_file);
  Multiplexer multiplexer{db, journal};
  Interpreter interpreter(db, multiplexer, nullptr, nullptr, 0);
  interpreter.set_echo(false);
  interpreter.set_rethrow(true);
  {
   std::ofstream null_stream;
   interpreter.main_loop(file, null_stream);
  }
  journal.default_checkpoint();
 }
}
