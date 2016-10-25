#ifndef joedb_File_declared
#define joedb_File_declared

#include "Generic_File.h"

#include <cstdio>

namespace joedb
{
 class File: public Generic_File
 {
  public:
   File(): file(0) {}
   File(const char *file_name, mode_t mode) {open(file_name, mode);}
   void open(const char *file_name, mode_t mode);
   ~File() override;

  protected:
   size_t read_buffer() override;
   void write_buffer() override;
   int seek(size_t offset) override;
   void sync() override;

  private:
   FILE *file;
   bool lock_file();
 };
}

#endif
