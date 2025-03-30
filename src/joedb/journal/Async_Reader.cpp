#include "joedb/journal/Async_Reader.h"
#include "joedb/journal/Buffer.h"

namespace joedb
{
 //////////////////////////////////////////////////////////////////////////
 Async_Reader::Async_Reader
 //////////////////////////////////////////////////////////////////////////
 (
  const Abstract_File &file,
  int64_t start,
  int64_t end
 ):
  file(file),
  end(end),
  current(start),
  end_of_file(false)
 {
  if (current > end)
   current = end;
 }

 //////////////////////////////////////////////////////////////////////////
 Async_Reader::Async_Reader
 //////////////////////////////////////////////////////////////////////////
 (
  const Abstract_File &file,
  Blob blob
 ):
  file(file)
 {
  Buffer<3> buffer;
  buffer.index = 0;
  buffer.write<int64_t>(0); // avoid uninitialized data if eof
  const size_t read = file.pread(buffer.data, 8, blob.get_position());
  buffer.index = 0;
  int64_t size = buffer.compact_read<int64_t>();
  end_of_file = read < buffer.index;
  current = blob.get_position() + int64_t(buffer.index);
  end = current + size;

  if (current > end)
   current = end;
 }

 //////////////////////////////////////////////////////////////////////////
 size_t Async_Reader::read(char *buffer, size_t capacity)
 //////////////////////////////////////////////////////////////////////////
 {
  size_t size = size_t(end - current);
  if (size > capacity)
   size = capacity;

  size_t total_read = 0;

  while (size > 0)
  {
   const size_t actually_read = file.pread
   (
    buffer + total_read,
    size,
    current
   );

   if (actually_read == 0)
   {
    end_of_file = true;
    break;
   }
   else
    end_of_file = false;

   current += int64_t(actually_read);
   total_read += actually_read;
   size -= actually_read;
  }

  return total_read;
 }
}
