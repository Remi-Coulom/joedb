#include "joedb/journal/Interpreted_File.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/interpreter/Database.h"
#include "joedb/Readable_Multiplexer.h"
#include "joedb/io/Interpreter.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Interpreted_File::Interpreted_File(std::istream &file)
 ////////////////////////////////////////////////////////////////////////////
 {
  Writable_Journal journal(*this);
  Database db;
  Readable_Multiplexer multiplexer(db);
  multiplexer.add_writable(journal);
  Interpreter interpreter(multiplexer);
  interpreter.set_echo(false);
  interpreter.set_rethrow(true);
  interpreter.main_loop(file, std::cout);
  journal.checkpoint(0);
  this->set_position(0);
  set_mode(Open_Mode::read_existing);
 }
}
