#include "joedb/journal/Async_Reader.h"

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
