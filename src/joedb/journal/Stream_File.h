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
   size_t raw_read(char *buffer, size_t size) override;
   void raw_write(const char *buffer, size_t size) override;
   void raw_seek(int64_t offset) override;

  public:
   Stream_File(std::streambuf &streambuf, Open_Mode mode);

   int64_t get_size() const override;

   ~Stream_File() override;
 };
}

#endif
