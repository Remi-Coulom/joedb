#ifndef joedb_Portable_File_declared
#define joedb_Portable_File_declared

#include "joedb/journal/Generic_File.h"

#include <cstdio>

namespace joedb
{
 ///////////////////////////////////////////////////////////////////////////
 class Portable_File: public Generic_File
 ///////////////////////////////////////////////////////////////////////////
 {
  private:
   bool try_open(const char *file_name, Open_Mode mode);
   std::FILE *file = nullptr;

  protected:
   Portable_File(std::FILE *file, Open_Mode mode):
    Generic_File(mode),
    file(file)
   {
   }

   size_t raw_read(char *buffer, size_t size) override;
   void raw_write(const char *buffer, size_t size) override;
   int seek(int64_t offset) override;
   void sync() override {}

  public:
   Portable_File(const char *file_name, Open_Mode mode);
   Portable_File(const std::string &file_name, Open_Mode mode):
    Portable_File(file_name.c_str(), mode)
   {
   }

   int64_t get_size() const override;

   ~Portable_File() override;
 };
}

#endif
