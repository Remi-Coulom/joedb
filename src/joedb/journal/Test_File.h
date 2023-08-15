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
   size_t raw_read(char *buffer, size_t size) override
   //////////////////////////////////////////////////////////////////////////
   {
    const size_t max_size = data.size() - current;
    const size_t n = std::min(size_t(read_size), std::min(size, max_size));
    std::copy_n(data.data() + current, n, buffer);
    current += n;
    return n;
   }

  public:
   Test_File(Open_Mode mode = Open_Mode::create_new):
    Memory_File(mode)
   {
   }
 };
}

#endif
