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
   bool end_of_file;

  public:
   //////////////////////////////////////////////////////////////////////////
   Async_Reader(Generic_File &file, int64_t start, int64_t end):
   //////////////////////////////////////////////////////////////////////////
    file(file),
    end(end),
    current(start),
    end_of_file(false)
   {
    file.flush();
    if (current > end)
     current = end;
   }

   //////////////////////////////////////////////////////////////////////////
   Async_Reader(Generic_File &file, Blob blob): file(file)
   //////////////////////////////////////////////////////////////////////////
   {
    file.flush();

    file.buffer.write<int64_t>(0); // avoid uninitialized data if eof
    const size_t read = file.pread(file.buffer.data, 8, blob.get_position());
    file.buffer.index = 0;
    int64_t size = file.buffer.compact_read<int64_t>();
    end_of_file = read < file.buffer.index;
    current = blob.get_position() + file.buffer.index;
    file.buffer.index = 0;
    end = current + size;

    if (current > end)
     current = end;
   }

   //////////////////////////////////////////////////////////////////////////
   size_t read(char *buffer, size_t capacity)
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

     current += actually_read;
     total_read += actually_read;
     size -= actually_read;
    }

    return total_read;
   }

   bool is_end_of_file() const {return end_of_file;}
   int64_t get_end() const {return end;}
   int64_t get_current() const {return current;}
   int64_t get_remaining() const {return end - current;}
 };
}

#endif
