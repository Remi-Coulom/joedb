#include "joedb/journal/Interpreted_File.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/Multiplexer.h"
#include "joedb/io/Interpreter.h"
#include "joedb/io/Interpreter_Dump_Writable.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Readonly_Interpreted_File::Readonly_Interpreted_File
 ////////////////////////////////////////////////////////////////////////////
 (
  std::istream &stream,
  bool readonly
 ):
  journal(*this)
 {
  stream.clear();
  stream.exceptions(std::ios::badbit);

  Multiplexer multiplexer{db, journal};
  Interpreter interpreter(db, multiplexer, nullptr, nullptr, 0);
  interpreter.set_echo(false);
  interpreter.set_rethrow(true);
  {
   std::ofstream null_stream;
   interpreter.main_loop(stream, null_stream);
  }
  journal.default_checkpoint();

  if (readonly)
   make_readonly();
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreted_Stream_File::pull()
 ////////////////////////////////////////////////////////////////////////////
 {
  const int64_t previous_checkpoint = journal.get_checkpoint_position();
  journal.pull();

  if (journal.get_checkpoint_position() > previous_checkpoint)
  {
   if (previous_checkpoint > Readonly_Journal::header_size)
    stream << '\n';
   Interpreter_Writable writable(stream, db);
   Multiplexer multiplexer{writable, db};
   journal.set_position(previous_checkpoint);
   journal.play_until_checkpoint(multiplexer);
   stream.flush();
  }
 }
}
