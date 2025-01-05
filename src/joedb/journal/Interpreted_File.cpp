#include "joedb/journal/Interpreted_File.h"
#include "joedb/Multiplexer.h"
#include "joedb/io/Interpreter_Dump_Writable.h"

namespace joedb
{
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

 ////////////////////////////////////////////////////////////////////////////
 void Interpreted_Stream_File::pwrite
 ////////////////////////////////////////////////////////////////////////////
 (
  const char *buffer,
  size_t size,
  int64_t offset
 )
 {
  Memory_File::pwrite(buffer, size, offset);
  if (Readonly_Journal::is_second_checkpoint_copy(offset))
   pull();
 }

 ////////////////////////////////////////////////////////////////////////////
 Interpreted_Stream_File::Interpreted_Stream_File(std::iostream &stream):
 ////////////////////////////////////////////////////////////////////////////
  Readonly_Interpreted_File(stream, false),
  stream(stream)
 {
  stream.clear(); // clears eof flag after reading, get ready to write
 }

 ////////////////////////////////////////////////////////////////////////////
 Interpreted_File_Data::Interpreted_File_Data
 ////////////////////////////////////////////////////////////////////////////
 (
  const char *file_name,
  Open_Mode mode
 ):
  file_stream(file_name, mode)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 Interpreted_File_Data::~Interpreted_File_Data() = default;
 ////////////////////////////////////////////////////////////////////////////

 ////////////////////////////////////////////////////////////////////////////
 Interpreted_File::Interpreted_File(const char *file_name, Open_Mode mode):
 ////////////////////////////////////////////////////////////////////////////
  Interpreted_File_Data(file_name, mode),
  Interpreted_Stream_File(file_stream)
 {
 }
}
