#ifndef joedb_filebuf_declared
#define joedb_filebuf_declared

#include "joedb/journal/Generic_File.h"

#include <streambuf>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class filebuf: public std::streambuf
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   using Traits = std::char_traits<char>;

   Generic_File &file;
   int64_t get_position;
   int64_t put_position;

  protected:
   // TODO: seekoff xsgetn
   pos_type seekpos
   (
    pos_type pos,
    std::ios_base::openmode which = std::ios_base::in | std::ios_base::out
   ) override;

   int_type underflow() override;
   std::streamsize xsputn(const char* s, std::streamsize count) override;
   int_type overflow(int_type ch = Traits::eof()) override;
   int sync() override;

  public:
   filebuf(Generic_File &file):
    file(file),
    get_position(0),
    put_position(0)
   {
    const int64_t size = file.get_size();
    if (size > 0)
     put_position = size;
   }
 };
}

#endif
