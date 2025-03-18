#ifndef joedb_Async_Reader_declared
#define joedb_Async_Reader_declared

#include "joedb/journal/Generic_File.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Async_Reader
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Generic_File &file;
   int64_t end;
   int64_t current;

  public:
   //////////////////////////////////////////////////////////////////////////
   Async_Reader(Generic_File &file, int64_t start, int64_t end):
   //////////////////////////////////////////////////////////////////////////
    file(file),
    end(end),
    current(start)
   {
    if (current > end)
     current = end;
    file.flush();
   }

   //////////////////////////////////////////////////////////////////////////
   Async_Reader(Generic_File &file, Blob blob): file(file)
   //////////////////////////////////////////////////////////////////////////
   {
    const int64_t original_position = file.get_position();

    file.set_position(blob.get_position());
    const int64_t size = file.compact_read<int64_t>();
    current = file.get_position();
    end = current + size;

    file.set_position(original_position);
   }

   //////////////////////////////////////////////////////////////////////////
   size_t read(char *buffer, size_t capacity)
   //////////////////////////////////////////////////////////////////////////
   {
    int64_t size = end - current;
    if (size > int64_t(capacity))
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
      break;

     current += actually_read;
     total_read += actually_read;
     size -= actually_read;
    }

    return total_read;
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t get_remaining() const
   //////////////////////////////////////////////////////////////////////////
   {
    return end - current;
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t get_end() const
   //////////////////////////////////////////////////////////////////////////
   {
    return end;
   }
 };
}

#endif
