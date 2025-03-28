#ifndef joedb_Memory_File_declared
#define joedb_Memory_File_declared

#include "joedb/journal/Buffered_File.h"

#include <string>

namespace joedb
{
 /// \ingroup journal
 class Memory_File: public Buffered_File
 {
  protected:
   std::string data;

  public:
   Memory_File(): Buffered_File(Open_Mode::create_new) {}

   void resize(size_t size) {data.resize(size);}
   std::string &get_data() {return data;}
   const std::string &get_data() const {return data;}
   std::string move_data() const {return std::move(data);}

   int64_t get_size() const override {return int64_t(data.size());}
   size_t pread(char *buffer, size_t size, int64_t offset) const override;
   void pwrite(const char *buffer, size_t size, int64_t offset) override;

   ~Memory_File() override;
 };
}

#endif
