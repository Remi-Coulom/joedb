#include "joedb/journal/Interpreted_File.h"
#include "joedb/journal/Readonly_Memory_File.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/Multiplexer.h"
#include "joedb/io/Interpreter.h"
#include "joedb/io/Interpreter_Dump_Writable.h"
#include "joedb/interpreter/Database.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Readonly_Interpreted_File::Readonly_Interpreted_File
 ////////////////////////////////////////////////////////////////////////////
 (
  std::istream &stream,
  bool readonly
 )
 {
  stream.clear();

  Writable_Journal journal(*this);
  Multiplexer multiplexer{db, journal};
  Interpreter interpreter(db, multiplexer, nullptr, nullptr, 0);
  interpreter.set_echo(false);
  interpreter.set_rethrow(true);
  {
   std::ofstream null_stream;
   interpreter.main_loop(stream, null_stream);
  }
  journal.default_checkpoint();

  current_checkpoint = journal.get_checkpoint_position();

  if (readonly)
   make_readonly();

  stream.clear(); // clears eof flag, get ready to write
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreted_Stream::pull()
 ////////////////////////////////////////////////////////////////////////////
 {
  Readonly_Memory_File file(get_data());
  Readonly_Journal journal(file);

  if (journal.get_checkpoint_position() > current_checkpoint)
  {
   journal.set_position(current_checkpoint);
   Interpreter_Writable writable(stream, db);
   Multiplexer multiplexer{writable, db};
   journal.play_until_checkpoint(multiplexer);
   stream.flush();
   current_checkpoint = journal.get_checkpoint_position();
  }
 }
}
