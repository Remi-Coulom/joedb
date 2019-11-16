#ifndef joedb_Stream_File_declared
#define joedb_Stream_File_declared

#include "Generic_File.h"

#include <iostream>

namespace joedb
{
 class Input_Stream_File: public Generic_File
 {
  public:
   Input_Stream_File(std::istream &stream);
   int64_t get_size() const override;

  protected:
   size_t read_buffer() override;
   void write_buffer() override {}
   int seek(int64_t offset) override;
   void sync() override {}

  private:
   std::istream &stream;
 };

 class Stream_File: public Generic_File
 {
  public:
   Stream_File(std::iostream &stream, Open_Mode mode);
   int64_t get_size() const override;

  protected:
   size_t read_buffer() override;
   void write_buffer() override;
   int seek(int64_t offset) override;
   void sync() override {}

  private:
   std::iostream &stream;
 };
}

#endif
