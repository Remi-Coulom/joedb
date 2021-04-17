#ifndef joedb_Async_Writer_declared
#define joedb_Async_Writer_declared

#include "joedb/journal/Generic_File.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Async_Writer
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Generic_File &file;
   int64_t current;

  public:
   //////////////////////////////////////////////////////////////////////////
   Async_Writer(Generic_File &file, int64_t start):
   //////////////////////////////////////////////////////////////////////////
    file(file),
    current(start)
   {
   }

   //////////////////////////////////////////////////////////////////////////
   void write(const char *buffer, size_t size)
   //////////////////////////////////////////////////////////////////////////
   {
    file.seek(current);
    file.raw_write(buffer, size);
    current += size;
   }
 };
}

#endif
