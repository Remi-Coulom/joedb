#ifndef joedb_Memory_File_declared
#define joedb_Memory_File_declared

#include "joedb/journal/Generic_File.h"

#include <algorithm>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Plain_Memory_File_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   std::vector<char> data;
   Plain_Memory_File_Data(std::vector<char> &data);
   Plain_Memory_File_Data();
   ~Plain_Memory_File_Data();
 };

 template<typename Parent>
 ////////////////////////////////////////////////////////////////////////////
 class Memory_File_Template: public Parent, public Generic_File
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   //////////////////////////////////////////////////////////////////////////
   size_t pread(char *buffer, size_t size, int64_t offset) override
   //////////////////////////////////////////////////////////////////////////
   {
    const size_t max_size = Parent::data.size() - offset;
    const size_t n = std::min(size, max_size);
    std::copy_n(Parent::data.data() + offset, n, buffer);
    return n;
   }

   //////////////////////////////////////////////////////////////////////////
   void pwrite(const char *buffer, size_t size, int64_t offset) override
   //////////////////////////////////////////////////////////////////////////
   {
    const size_t end = offset + size;
    if (end > Parent::data.size())
     Parent::data.resize(end);
    std::copy_n(buffer, size, &Parent::data[offset]);
   }

  public:
   //////////////////////////////////////////////////////////////////////////
   Memory_File_Template():
   //////////////////////////////////////////////////////////////////////////
    Generic_File(Open_Mode::create_new)
   {
   }

   //////////////////////////////////////////////////////////////////////////
   Memory_File_Template(std::vector<char> &data):
   //////////////////////////////////////////////////////////////////////////
    Parent(data),
    Generic_File(Open_Mode::shared_write)
   {
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t get_size() const override
   //////////////////////////////////////////////////////////////////////////
   {
    return int64_t(Parent::data.size());
   }

   //////////////////////////////////////////////////////////////////////////
   const std::vector<char> &get_data() const
   //////////////////////////////////////////////////////////////////////////
   {
    return Parent::data;
   }

   //////////////////////////////////////////////////////////////////////////
   ~Memory_File_Template() override
   //////////////////////////////////////////////////////////////////////////
   {
    destructor_flush();
   }
 };

 extern template class Memory_File_Template<Plain_Memory_File_Data>;

 using Memory_File = Memory_File_Template<Plain_Memory_File_Data>;
}

#endif
