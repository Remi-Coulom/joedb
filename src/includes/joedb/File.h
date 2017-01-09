#ifndef joedb_File_declared
#define joedb_File_declared

#include "Generic_File.h"

#include <cstdio>

namespace joedb
{
 class File: public Generic_File
 {
  public:
   File(const char *file_name, Open_Mode mode);
   ~File() override;
   int64_t get_size() const override;

  protected:
   size_t read_buffer() override;
   void write_buffer() override;
   int seek(size_t offset) override;
   void sync() override;

  private:
   bool try_open(const char *file_name, Open_Mode mode);
   FILE *file = 0;
   bool lock_file();
   void close_file();
 };
}

#endif
