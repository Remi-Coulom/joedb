#include "joedb/journal/File_Continuation.h"
#include "joedb/journal/Readonly_Journal.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void File_Continuation::raw_sync()
 ////////////////////////////////////////////////////////////////////////////
 {
  continuation.raw_sync();
 }

 ////////////////////////////////////////////////////////////////////////////
 void File_Continuation::shared_lock(int64_t start, int64_t size)
 ////////////////////////////////////////////////////////////////////////////
 {
  continuation.shared_lock(start, size);
 }

 ////////////////////////////////////////////////////////////////////////////
 void File_Continuation::exclusive_lock(int64_t start, int64_t size)
 ////////////////////////////////////////////////////////////////////////////
 {
  continuation.exclusive_lock(start, size);
 }

 ////////////////////////////////////////////////////////////////////////////
 void File_Continuation::unlock(int64_t start, int64_t size)
 ////////////////////////////////////////////////////////////////////////////
 {
  continuation.unlock(start, size);
 }

 ////////////////////////////////////////////////////////////////////////////
 File_Continuation::File_Continuation
 ////////////////////////////////////////////////////////////////////////////
 (
  Generic_File &file,
  Generic_File &continuation,
  int64_t continuation_offset
 ):
  Generic_File(continuation.mode),
  file(file),
  continuation(continuation),
  continuation_offset(continuation_offset)
 {
  continuation.set_position(0);

  if (continuation.get_size() == 0)
  {
   continuation.write<int64_t>(continuation_offset);
   char buffer[Readonly_Journal::header_size];
   const int64_t size = file.read_data(buffer, Readonly_Journal::header_size);
   if (size < Readonly_Journal::header_size)
    throw Runtime_Error("File_Continuation requires a full header");
   continuation.write_data(buffer, Readonly_Journal::header_size);
  }
  else
  {
   const int64_t file_offset = continuation.read<int64_t>();
   if (file_offset != continuation_offset)
    throw Runtime_Error("Bad continuation offset");
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 size_t File_Continuation::pread(char *data, size_t size, int64_t offset)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (offset < Readonly_Journal::header_size)
  {
   if (offset + size > Readonly_Journal::header_size)
    size = Readonly_Journal::header_size - offset;

   return continuation.pread(data, size, offset + 8);
  }

  if (offset < continuation_offset)
  {
   if (offset + int64_t(size) > continuation_offset)
    size = continuation_offset - offset;

   return file.pread(data, size, offset);
  }

  return continuation.pread(data, size, offset + 8 - continuation_offset);
 }

 ////////////////////////////////////////////////////////////////////////////
 void File_Continuation::pwrite(const char *data, size_t size, int64_t offset)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (offset < continuation_offset)
   throw Runtime_Error("Cannot write before continuation offset");

  continuation.pwrite(data, size, offset + 8 - continuation_offset);
 }
}
