#ifndef joedb_Memory_File_declared
#define joedb_Memory_File_declared

#include "joedb/journal/Generic_File.h"

#include <vector>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Memory_File: public Generic_File
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   std::vector<char> data;
   size_t pread(char *buffer, size_t size, int64_t offset) override;
   void pwrite(const char *buffer, size_t size, int64_t offset) override;

  public:
   Memory_File(): Generic_File(Open_Mode::create_new) {}

   int64_t get_size() const override {return int64_t(data.size());}
   const std::vector<char> &get_data() const {return data;}

   ~Memory_File() override;
 };
}

#endif
