#ifndef joedb_streambuf_declared
#define joedb_streambuf_declared

#include <streambuf>

#include <array>
#include <cstring>

#include "joedb/journal/Abstract_File.h"
#include "joedb/error/Destructor_Logger.h"

namespace joedb
{
 class streambuf: public std::streambuf
 {
  private:
   Abstract_File &file;

   static constexpr size_t buffer_size = (1 << 13);
   std::array<char, buffer_size> buffer;

   pos_type in_pos;
   pos_type out_pos;

   void syncg()
   {
    in_pos += gptr() - eback();
    setg(buffer.data(), buffer.data(), buffer.data());
   }

   void syncp()
   {
    const size_t n = pptr() - pbase();
    if (n)
    {
     file.pwrite(buffer.data(), n, out_pos);
     out_pos += n;
     setp(buffer.data(), buffer.data() + buffer_size);
    }
   }

  protected:
   pos_type seekoff
   (
    off_type off,
    std::ios_base::seekdir dir,
    std::ios_base::openmode which
   ) override
   {
    pos_type base = -1;

    if (dir == std::ios_base::beg)
     base = 0;
    else if (dir == std::ios_base::cur)
    {
     base = (which & std::ios_base::in)
      ? in_pos + gptr() - eback()
      : out_pos + pptr() - pbase();
    }
    else if (dir == std::ios_base::end)
     base = file.get_size();

    if (base < 0)
     return pos_type(off_type(-1));
    else
     return seekpos(base + off, which);
   }

   pos_type seekpos(pos_type pos, std::ios_base::openmode which) override
   {
    sync();

    if (which & std::ios_base::in)
     in_pos = pos;

    if (which & std::ios_base::out)
     out_pos = pos;

    return pos;
   }

   int sync() override
   {
    syncg();
    syncp();
    return 0;
   }

   std::streamsize showmanyc() override
   {
    const int64_t size = file.get_size();
    if (size < 0)
     return 0;
    return size - (in_pos + gptr() - eback());
   }

   int_type underflow() override
   {
    if (gptr() == egptr())
    {
     in_pos += gptr() - eback();
     const size_t size = file.pread(buffer.data(), buffer.size(), in_pos);
     setg(buffer.data(), buffer.data(), buffer.data() + size);
     if (size == 0)
      return traits_type::eof();
    }

    return traits_type::to_int_type(*gptr());
   }

   int_type uflow() override
   {
    const int_type c = underflow();
    if (!traits_type::eq_int_type(c, traits_type::eof()))
     gbump(1);
    return c;
   }

   std::streamsize xsgetn(char_type* s, std::streamsize count) override
   {
    std::streamsize result = 0;

    {
     const std::streamsize available = egptr() - gptr();
     const std::streamsize n = std::min(available, count);
     std::memcpy(s, gptr(), n);
     gbump(n);
     result += n;
     s += n;
     count -= n;
    }

    if (count > 0)
    {
     syncg();

     do
     {
      const size_t n = file.pread(s, count, in_pos);
      if (n > 0)
      {
       in_pos += n;
       result += n;
       s += n;
       count -= n;
      }
      else
       break;
     }
     while (count > 0);
    }

    return result;
   }

   std::streamsize xsputn(const char_type* s, std::streamsize count) override
   {
    const ptrdiff_t available = epptr() - pptr();

    if (available >= count)
    {
     std::memcpy(pptr(), s, count);
     pbump(count);
    }
    else
    {
     syncp();
     file.pwrite(s, count, out_pos);
     out_pos += count;
    }

    return count;
   }

   int_type overflow(int_type ch) override
   {
    if (pptr() >= epptr())
     syncp();

    if (!traits_type::eq_int_type(ch, traits_type::eof()))
    {
     *pptr() = traits_type::to_char_type(ch);
     pbump(1);
    }

    return traits_type::not_eof(ch);
   }

   int_type pbackfail(int_type c) override
   {
    if (gptr() == eback() && in_pos > 0)
    {
     in_pos -= 1;
     setg(buffer.data(), buffer.data() + 1, buffer.data() + 1);
     file.pread(buffer.data(), 1, in_pos);
    }

    if (gptr() > eback())
    {
     gbump(-1);
     if (!traits_type::eq_int_type(c, traits_type::eof()))
     {
      *gptr() = traits_type::to_char_type(c);
      file.pwrite(gptr(), 1, in_pos + gptr() - eback());
     }
     return *gptr();
    }

    return traits_type::eof();
   }

  public:
   streambuf(Abstract_File &file): file(file)
   {
    in_pos = 0;
    setg(buffer.data(), buffer.data(), buffer.data());

    out_pos = 0;
    setp(buffer.data(), buffer.data() + buffer_size);
   }

   ~streambuf() override
   {
    if (pptr() > pbase())
    {
     Destructor_Logger::warning("streambuf: flushing buffer");
     try {syncp();} catch (...) {}
    }
   }
 };
}

#endif
