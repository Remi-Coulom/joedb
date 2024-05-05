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
   const int64_t end;
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
   size_t read(char *buffer, size_t capacity)
   //////////////////////////////////////////////////////////////////////////
   {
    int64_t size = end - current;
    if (size > int64_t(capacity))
     size = capacity;
    const size_t actually_read = size > 0
     ? file.pos_pread(buffer, size, current)
     : 0;
    current += actually_read;
    return actually_read;
   }

   //////////////////////////////////////////////////////////////////////////
   size_t pos_pread(char *buffer, size_t capacity, int64_t offset)
   //////////////////////////////////////////////////////////////////////////
   {
    return file.pos_pread(buffer, capacity, offset);
   }

   //////////////////////////////////////////////////////////////////////////
   size_t seek_and_read(char *buffer, size_t capacity, int64_t offset)
   //////////////////////////////////////////////////////////////////////////
   {
    file.seek(offset);
    return file.pos_read(buffer, capacity);
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
