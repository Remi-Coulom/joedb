#include "joedb/journal/Stream_File.h"
#include "joedb/error/Exception.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 Stream_File::Stream_File
 /////////////////////////////////////////////////////////////////////////////
 (
  std::streambuf &streambuf,
  Open_Mode mode
 ):
  Abstract_File(mode),
  streambuf(streambuf)
 {
  streambuf.pubseekoff
  (
   0,
   std::ios_base::beg,
   std::ios_base::in
  );
 }

 /////////////////////////////////////////////////////////////////////////////
 int64_t Stream_File::get_size() const
 /////////////////////////////////////////////////////////////////////////////
 {
  return streambuf.pubseekoff
  (
   0,
   std::ios_base::end,
   std::ios_base::in
  );
 }

 /////////////////////////////////////////////////////////////////////////////
 void Stream_File::seek
 /////////////////////////////////////////////////////////////////////////////
 (
  int64_t offset,
  std::ios_base::openmode which
 ) const
 {
  if (offset >= 0)
  {
   const auto pos = streambuf.pubseekoff(offset, std::ios_base::beg, which);
   if (int64_t(pos) == offset)
    return;
  }

  throw Exception("seek error");
 }

 /////////////////////////////////////////////////////////////////////////////
 size_t Stream_File::pread(char *data, size_t size, int64_t offset) const
 /////////////////////////////////////////////////////////////////////////////
 {
  seek(offset, std::ios_base::in);
  const std::streamsize n = streambuf.sgetn(data, std::streamsize(size));
  return size_t(n);
 }

 /////////////////////////////////////////////////////////////////////////////
 void Stream_File::pwrite(const char *data, size_t size, int64_t offset)
 /////////////////////////////////////////////////////////////////////////////
 {
  seek(offset, std::ios_base::out);

  size_t written = 0;

  while (written < size)
  {
   const std::streamsize n = streambuf.sputn
   (
    data + written,
    std::streamsize(size - written)
   );

   if (n <= 0)
    throw Exception("Could not write to stream");

   written += size_t(n);
  }

  if (streambuf.pubsync() < 0)
   throw Exception("sync error");
 }
}
