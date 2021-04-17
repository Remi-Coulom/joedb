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
   }

   //////////////////////////////////////////////////////////////////////////
   size_t read(char *buffer, size_t capacity)
   //////////////////////////////////////////////////////////////////////////
   {
    size_t size = size_t(end - current);

    if (size > 0)
    {
     if (size > capacity)
      size = capacity;

     file.seek(current);
     file.raw_read(buffer, size);

     current += size;
    }

    return size;
   }
 };
}

#endif
