#include "joedb/journal/Interpreted_File.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/interpreter/Database.h"
#include "joedb/Multiplexer.h"
#include "joedb/io/Interpreter.h"

#include <iostream>
#include <fstream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Interpreted_File::Interpreted_File(std::istream &file)
 ////////////////////////////////////////////////////////////////////////////
 {
  Writable_Journal journal(*this);
  Database db;
  Multiplexer multiplexer{db, journal};
  Interpreter interpreter(db, multiplexer);
  interpreter.set_echo(false);
  interpreter.set_rethrow(true);
  {
   std::ofstream null_stream;
   interpreter.main_loop(file, null_stream);
  }
  journal.checkpoint(Commit_Level::no_commit);
  set_mode(Open_Mode::write_existing);
 }
}
