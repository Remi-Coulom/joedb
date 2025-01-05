#include "joedb/journal/Readonly_Interpreted_File.h"
#include "joedb/Multiplexer.h"
#include "joedb/io/Interpreter.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Readonly_Interpreted_File::Readonly_Interpreted_File
 ////////////////////////////////////////////////////////////////////////////
 (
  std::istream &stream,
  Open_Mode mode
 ):
  journal(*this)
 {
  Multiplexer multiplexer{db, journal};
  Interpreter interpreter(db, multiplexer, nullptr, multiplexer, 0);
  interpreter.set_echo(false);
  interpreter.set_rethrow(true);
  {
   joedb::dummy_stream null_stream;
   interpreter.main_loop(stream, null_stream);
  }
  journal.default_checkpoint();

  set_mode(mode);
 }

 ////////////////////////////////////////////////////////////////////////////
 Readonly_Interpreted_File::~Readonly_Interpreted_File() = default;
 ////////////////////////////////////////////////////////////////////////////
}
