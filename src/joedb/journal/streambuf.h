#ifndef joedb_streambuf_declared
#define joedb_streambuf_declared

#include "joedb/journal/Abstract_File.h"

#include <streambuf>
#include <array>
#include <cstring>

namespace joedb
{
 ///
 /// https://en.cppreference.com/w/cpp/io/basic_streambuf.html
 ///
 /// @ingroup journal
 class streambuf: public std::streambuf
 {
  private:
   Abstract_File &file;

   static constexpr size_t buffer_size = (1 << 13);
   std::array<char, buffer_size> buffer;

   pos_type in_pos;
   pos_type out_pos;

   void syncg();
   void syncp();

  protected:
   pos_type seekoff
   (
    off_type off,
    std::ios_base::seekdir dir,
    std::ios_base::openmode which
   ) override;

   pos_type seekpos(pos_type pos, std::ios_base::openmode which) override;
   int sync() override;
   std::streamsize showmanyc() override;
   int_type underflow() override;
   int_type uflow() override;
   std::streamsize xsgetn(char_type* s, std::streamsize count) override;
   std::streamsize xsputn(const char_type* s, std::streamsize count) override;
   int_type overflow(int_type ch) override;
   int_type pbackfail(int_type c) override;

  public:
   streambuf(Abstract_File &file);
   ~streambuf() override;
 };
}

#endif
