#include "joedb/journal/Interpreted_File.h"
#include "joedb/Multiplexer.h"
#include "joedb/ui/Interpreter_Dump_Writable.h"

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
   Interpreter_Writable writable(stream, db);
   Multiplexer multiplexer{writable, db};
   readonly_journal.pull();
   readonly_journal.play_until_checkpoint(multiplexer);
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
  if (offset < Header::ssize)
   pull();
 }

 ////////////////////////////////////////////////////////////////////////////
 Interpreted_Stream_File::Interpreted_Stream_File(std::iostream &stream):
 ////////////////////////////////////////////////////////////////////////////
  Readonly_Interpreted_File(stream, false),
  stream(stream),
  file_view(*this),
  readonly_journal(file_view)
 {
  stream.clear(); // clears eof flag after reading, get ready to write
  Writable writable;
  readonly_journal.play_until_checkpoint(writable);
 }

 ////////////////////////////////////////////////////////////////////////////
 detail::Interpreted_File_Data::Interpreted_File_Data(const char *file_name)
 ////////////////////////////////////////////////////////////////////////////
 {
  constexpr auto in = std::ios::binary | std::ios::in;
  file_stream.open(file_name, in | std::ios::out);
  if (!file_stream)
   file_stream.open(file_name, in | std::ios::out | std::ios::trunc);
 }

 ////////////////////////////////////////////////////////////////////////////
 detail::Interpreted_File_Data::~Interpreted_File_Data() = default;
 ////////////////////////////////////////////////////////////////////////////

 ////////////////////////////////////////////////////////////////////////////
 Interpreted_File::Interpreted_File(const char *file_name):
 ////////////////////////////////////////////////////////////////////////////
  Interpreted_File_Data(file_name),
  Interpreted_Stream_File(file_stream)
 {
 }
}
