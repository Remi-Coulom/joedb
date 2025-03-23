#ifndef joedb_Stream_File_declared
#define joedb_Stream_File_declared

#include "joedb/journal/Generic_File.h"

#include <streambuf>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Stream_File: public Generic_File
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::streambuf &streambuf;
   mutable std::streambuf::pos_type pos;
   void seek(int64_t offset);

  public:
   Stream_File(std::streambuf &streambuf, Open_Mode mode);

   size_t pread(char *data, size_t size, int64_t offset) override;
   void pwrite(const char *data, size_t size, int64_t offset) override;
   int64_t get_size() const override;

   ~Stream_File() override;
 };
}

#endif
