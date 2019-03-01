#ifndef joedb_File_declared
#define joedb_File_declared

#include "Generic_File.h"

#include <stdio.h>

namespace joedb
{
 ///////////////////////////////////////////////////////////////////////////
 class File: public Generic_File
 ///////////////////////////////////////////////////////////////////////////
 {
  public:
   File(const char *file_name, Open_Mode mode);
   File(const std::string &file_name, Open_Mode mode):
    File(file_name.c_str(), mode)
   {
   }

   ~File() override;
   int64_t get_size() const override;

  protected:
   File(FILE *file): file(file) {}

   size_t read_buffer() override;
   void write_buffer() override;
   int seek(size_t offset) override;
   void sync() override;

  private:
   bool try_open(const char *file_name, Open_Mode mode);
   FILE *file = nullptr;
   bool lock_file();
   void close_file();
 };

 ///////////////////////////////////////////////////////////////////////////
 class File_Slice: public File
 ///////////////////////////////////////////////////////////////////////////
 {
  public:
   File_Slice(FILE *file, size_t start, size_t length):
    File(file),
    start(start),
    length(length)
   {
    seek(0);
   }

   ~File_Slice() override {}

  protected:
   int64_t get_size() const override {return int64_t(length);}
   int seek(size_t offset) override {return File::seek(offset + start);}

  private:
   const size_t start;
   const size_t length;
 };
}

#endif
