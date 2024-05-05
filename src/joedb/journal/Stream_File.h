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

  protected:
   size_t raw_read(char *buffer, size_t size) final;
   void raw_write(const char *buffer, size_t size) final;
   void raw_seek(int64_t offset) final;
   int64_t raw_get_size() const final;

  public:
   Stream_File(std::streambuf &streambuf, Open_Mode mode);
   ~Stream_File();
 };
}

#endif
