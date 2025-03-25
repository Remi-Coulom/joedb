#ifndef joedb_Shared_Memory_File_declared
#define joedb_Shared_Memory_File_declared

#include "joedb/journal/Buffered_File.h"

#include <algorithm>
#include <vector>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Shared_Memory_File: public Buffered_File
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   std::vector<char> &data;

   //////////////////////////////////////////////////////////////////////////
   size_t pread(char *buffer, size_t size, int64_t offset) override
   //////////////////////////////////////////////////////////////////////////
   {
    if (offset < 0 || offset >= int64_t(data.size()))
     return 0;
    const size_t max_size = data.size() - size_t(offset);
    const size_t n = std::min(size, max_size);
    std::copy_n(data.data() + size_t(offset), n, buffer);
    return n;
   }

   //////////////////////////////////////////////////////////////////////////
   void pwrite(const char *buffer, size_t size, int64_t offset) override
   //////////////////////////////////////////////////////////////////////////
   {
    const size_t end = size_t(offset) + size;
    if (end > data.size())
     data.resize(end);
    std::copy_n(buffer, size, &data[size_t(offset)]);
   }

  public:
   //////////////////////////////////////////////////////////////////////////
   Shared_Memory_File(std::vector<char> &data):
   //////////////////////////////////////////////////////////////////////////
    Buffered_File(Open_Mode::shared_write),
    data(data)
   {
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t get_size() const override
   //////////////////////////////////////////////////////////////////////////
   {
    return int64_t(data.size());
   }

   //////////////////////////////////////////////////////////////////////////
   const std::vector<char> &get_data() const
   //////////////////////////////////////////////////////////////////////////
   {
    return data;
   }

   //////////////////////////////////////////////////////////////////////////
   ~Shared_Memory_File() override
   //////////////////////////////////////////////////////////////////////////
   {
    destructor_flush();
   }
 };
}

#endif
