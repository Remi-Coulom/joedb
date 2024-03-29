#include "joedb/journal/Interpreted_File.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/interpreter/Database.h"
#include "joedb/Multiplexer.h"
#include "joedb/io/Interpreter.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void append_interpreted_commands
 ////////////////////////////////////////////////////////////////////////////
 (
  joedb::Generic_File &joedb_file,
  std::istream &command_stream
 )
 {
  if (!command_stream.good())
   throw Exception("bad command stream");

  {
   joedb_file.flush();
   Writable_Journal journal(joedb_file);
   Database db;
   journal.replay_log(db);
   Multiplexer multiplexer{db, journal};
   Interpreter interpreter(db, multiplexer, nullptr, nullptr, 0);
   interpreter.set_echo(false);
   interpreter.set_rethrow(true);
   {
    std::ofstream null_stream;
    interpreter.main_loop(command_stream, null_stream);
   }
   journal.default_checkpoint();
  }

  joedb_file.set_mode(Open_Mode::write_existing);
 }

 ////////////////////////////////////////////////////////////////////////////
 Interpreted_File::Interpreted_File(std::istream &file)
 ////////////////////////////////////////////////////////////////////////////
 {
  append_interpreted_commands(*this, file);
 }
}
