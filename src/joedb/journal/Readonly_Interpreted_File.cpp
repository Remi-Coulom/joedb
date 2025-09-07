#include "joedb/journal/Readonly_Interpreted_File.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/Multiplexer.h"
#include "joedb/ui/Interpreter.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Readonly_Interpreted_File::Readonly_Interpreted_File
 ////////////////////////////////////////////////////////////////////////////
 (
  std::istream &stream,
  bool readonly
 ):
  file_view(*this),
  journal(file_view)
 {
  if (!stream)
   throw Exception("opening interpreted file: !stream");

  stream.exceptions(std::ios::badbit);
  stream.seekg(0);

  Multiplexer multiplexer{db, journal};
  Interpreter interpreter(db, multiplexer, Record_Id::null);
  interpreter.set_echo(false);
  interpreter.set_rethrow(true);
  {
   joedb::Abstract_File null_file;
   joedb::streambuf null_streambuf(null_file);
   std::ostream null_stream(&null_streambuf);
   interpreter.main_loop(stream, null_stream);
  }
  journal.soft_checkpoint();

  if (readonly)
   make_readonly();
 }

 ////////////////////////////////////////////////////////////////////////////
 Readonly_Interpreted_File::~Readonly_Interpreted_File() = default;
 ////////////////////////////////////////////////////////////////////////////
}
