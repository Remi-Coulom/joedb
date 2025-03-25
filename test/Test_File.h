#ifndef joedb_Test_File_declared
#define joedb_Test_File_declared

#include "joedb/journal/Memory_File.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Test_File: public Memory_File
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   enum {read_size = 1};

   //////////////////////////////////////////////////////////////////////////
   size_t pread(char *buffer, size_t size, int64_t offset) override
   //////////////////////////////////////////////////////////////////////////
   {
    const size_t max_size = data.size() - offset;
    const size_t n = std::min(size_t(read_size), std::min(size, max_size));
    std::copy_n(data.data() + offset, n, buffer);
    return n;
   }
 };
}

#endif
