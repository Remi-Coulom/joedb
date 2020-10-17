#ifndef joedb_Posix_File_declared
#define joedb_Posix_File_declared

#include "joedb/journal/Generic_File.h"

#include <stdio.h>

namespace joedb
{
 ///////////////////////////////////////////////////////////////////////////
 class Posix_File: public Generic_File
 ///////////////////////////////////////////////////////////////////////////
 {
  public:
   Posix_File(const char *file_name, Open_Mode mode);
   Posix_File(const std::string &file_name, Open_Mode mode):
    Posix_File(file_name.c_str(), mode)
   {
   }

   ~Posix_File() override;
   int64_t get_size() const override;

  protected:
   size_t read_buffer() override;
   void write_buffer() override;
   int seek(int64_t offset) override;
   void sync() override;

  private:
   bool try_open(const char *file_name, Open_Mode mode);
   FILE *file = nullptr;
   bool lock_file();
   void close_file();
   int seek(int64_t offset, int origin) const;
   int64_t tell() const;
 };
}

#endif
