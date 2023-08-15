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
   const int64_t initial_position;
   const int64_t end;
   int64_t current;

  public:
   //////////////////////////////////////////////////////////////////////////
   Async_Reader(Generic_File &file, int64_t start, int64_t end):
   //////////////////////////////////////////////////////////////////////////
    file(file),
    initial_position(file.get_position()),
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
     const size_t actually_read = file.raw_read(buffer, size);

     current += actually_read;
     return actually_read;
    }

    return 0;
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t get_remaining() const
   //////////////////////////////////////////////////////////////////////////
   {
    return end - current;
   }

   //////////////////////////////////////////////////////////////////////////
   ~Async_Reader()
   //////////////////////////////////////////////////////////////////////////
   {
    try
    {
     file.seek(initial_position);
    }
    catch (...)
    {
    }
   }
 };
}

#endif
