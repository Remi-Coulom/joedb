#ifndef joedb_Readonly_Memory_File_declared
#define joedb_Readonly_Memory_File_declared

#include "joedb/journal/Generic_File.h"

#include <algorithm>
#include <vector>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Readonly_Memory_File: public Generic_File
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   const char * data;
   const size_t data_size;

   //////////////////////////////////////////////////////////////////////////
   size_t pread(char *buffer, size_t size, int64_t offset) override
   //////////////////////////////////////////////////////////////////////////
   {
    if (size_t(offset) >= data_size)
     return 0;
    const size_t max_size = data_size - size_t(offset);
    const size_t n = std::min(size, max_size);
    std::copy_n(&data[size_t(offset)], n, buffer);
    return n;
   }

  public:
   //////////////////////////////////////////////////////////////////////////
   Readonly_Memory_File(const void *memory, size_t size):
   //////////////////////////////////////////////////////////////////////////
    Generic_File(joedb::Open_Mode::read_existing),
    data((const char *)memory),
    data_size(size)
   {
   }

   //////////////////////////////////////////////////////////////////////////
   Readonly_Memory_File(const std::string &s):
   //////////////////////////////////////////////////////////////////////////
    Readonly_Memory_File(s.data(), s.size())
   {
   }

   //////////////////////////////////////////////////////////////////////////
   Readonly_Memory_File(const std::vector<char> &v):
   //////////////////////////////////////////////////////////////////////////
    Readonly_Memory_File(v.data(), v.size())
   {
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t get_size() const override
   //////////////////////////////////////////////////////////////////////////
   {
    return int64_t(data_size);
   }
 };
}

#endif
