#ifndef joedb_Stream_File_declared
#define joedb_Stream_File_declared

#include "Generic_File.h"

#include <iostream>

namespace joedb
{
 class Stream_File: public Generic_File
 {
  public:
   Stream_File(std::iostream &stream, mode_t mode);

  protected:
   size_t read_buffer() override;
   void write_buffer() override;
   int seek(size_t offset) override;
   void sync() override {}

  private:
   std::iostream &stream;
 };
}

#endif
