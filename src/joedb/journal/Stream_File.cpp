#include "joedb/journal/Stream_File.h"
#include "joedb/Exception.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 Stream_File::Stream_File(std::streambuf &streambuf, Open_Mode mode):
 /////////////////////////////////////////////////////////////////////////////
  Generic_File(mode),
  streambuf(streambuf)
 {
 }

 /////////////////////////////////////////////////////////////////////////////
 int64_t Stream_File::raw_get_size() const
 /////////////////////////////////////////////////////////////////////////////
 {
  const auto pos = streambuf.pubseekoff
  (
   0,
   std::ios_base::cur,
   std::ios_base::in
  );

  const auto result = streambuf.pubseekoff
  (
   0,
   std::ios_base::end,
   std::ios_base::in
  );

  streambuf.pubseekoff
  (
   pos,
   std::ios_base::beg,
   std::ios_base::in
  );

  return int64_t(result);
 }

 /////////////////////////////////////////////////////////////////////////////
 size_t Stream_File::raw_read(char *buffer, size_t size)
 /////////////////////////////////////////////////////////////////////////////
 {
  return size_t(streambuf.sgetn(buffer, size));
 }

 /////////////////////////////////////////////////////////////////////////////
 void Stream_File::raw_write(const char *buffer, size_t size)
 /////////////////////////////////////////////////////////////////////////////
 {
  size_t written = 0;

  while (written < size)
  {
   const ssize_t result = streambuf.sputn(buffer + written, size - written);
   if (result <= 0)
    throw Exception("Could not write to stream");
   written += size_t(result);
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 int Stream_File::raw_seek(int64_t offset)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (offset < 0)
   return 1;

  const auto pos = streambuf.pubseekoff(offset, std::ios_base::beg);
  return pos != offset;
 }
}
