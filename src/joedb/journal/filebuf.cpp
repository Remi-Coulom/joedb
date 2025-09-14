#include "joedb/journal/filebuf.h"
#include "joedb/error/Destructor_Logger.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void filebuf::syncg()
 ////////////////////////////////////////////////////////////////////////////
 {
  in_pos += gptr() - eback();
  setg(buffer.data(), buffer.data(), buffer.data());
 }

 ////////////////////////////////////////////////////////////////////////////
 void filebuf::syncp()
 ////////////////////////////////////////////////////////////////////////////
 {
  const ptrdiff_t n = pptr() - pbase();
  if (n)
  {
   file.pwrite(buffer.data(), n, out_pos);
   out_pos += n;
   setp(buffer.data(), buffer.data() + buffer_size);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 filebuf::pos_type filebuf::seekoff
 ////////////////////////////////////////////////////////////////////////////
 (
  off_type off,
  std::ios_base::seekdir dir,
  std::ios_base::openmode which
 )
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

 ////////////////////////////////////////////////////////////////////////////
 filebuf::pos_type filebuf::seekpos
 ////////////////////////////////////////////////////////////////////////////
 (
  pos_type pos,
  std::ios_base::openmode which
 )
 {
  sync();

  if (which & std::ios_base::in)
   in_pos = pos;

  if (which & std::ios_base::out)
   out_pos = pos;

  return pos;
 }

 ////////////////////////////////////////////////////////////////////////////
 int filebuf::sync()
 ////////////////////////////////////////////////////////////////////////////
 {
  syncg();
  syncp();
  return 0;
 }

 ////////////////////////////////////////////////////////////////////////////
 std::streamsize filebuf::showmanyc()
 ////////////////////////////////////////////////////////////////////////////
 {
  const int64_t size = file.get_size();
  if (size < 0)
   return 0;
  return size - (in_pos + gptr() - eback());
 }

 ////////////////////////////////////////////////////////////////////////////
 filebuf::int_type filebuf::underflow()
 ////////////////////////////////////////////////////////////////////////////
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

 ////////////////////////////////////////////////////////////////////////////
 std::streamsize filebuf::xsgetn(char_type* s, std::streamsize count)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::streamsize result = 0;

  {
   const std::streamsize available = egptr() - gptr();
   const std::streamsize n = std::min(available, count);
   std::memcpy(s, gptr(), n);
   gbump(int(n));
   result += n;
   s += n;
   count -= n;
  }

  if (count > 0)
  {
   syncg();

   do
   {
    const std::streamsize n = std::streamsize(file.pread(s, count, in_pos));
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

 ////////////////////////////////////////////////////////////////////////////
 std::streamsize filebuf::xsputn(const char_type* s, std::streamsize count)
 ////////////////////////////////////////////////////////////////////////////
 {
  const ptrdiff_t available = epptr() - pptr();

  if (available >= count)
  {
   std::memcpy(pptr(), s, count);
   pbump(int(count));
  }
  else
  {
   syncp();
   file.pwrite(s, count, out_pos);
   out_pos += count;
  }

  return count;
 }

 ////////////////////////////////////////////////////////////////////////////
 filebuf::int_type filebuf::overflow(int_type ch)
 ////////////////////////////////////////////////////////////////////////////
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

 ////////////////////////////////////////////////////////////////////////////
 filebuf::int_type filebuf::pbackfail(int_type c)
 ////////////////////////////////////////////////////////////////////////////
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
    const char cc = traits_type::to_char_type(c);
    if (*gptr() != cc)
    {
     *gptr() = cc;
     file.pwrite(gptr(), 1, in_pos + gptr() - eback());
    }
   }
   return *gptr();
  }

  return traits_type::eof();
 }

 ////////////////////////////////////////////////////////////////////////////
 filebuf::filebuf(Abstract_File &file): file(file)
 ////////////////////////////////////////////////////////////////////////////
 {
  in_pos = 0;
  setg(buffer.data(), buffer.data(), buffer.data());

  out_pos = 0;
  setp(buffer.data(), buffer.data() + buffer_size);
 }

 ////////////////////////////////////////////////////////////////////////////
 filebuf::~filebuf()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (pptr() > pbase())
  {
   Destructor_Logger::warning("filebuf: flushing buffer");
   try {syncp();} catch (...) {}
  }
 }
}
