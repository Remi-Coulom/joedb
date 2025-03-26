#include "joedb/journal/Stream_File.h"
#include "joedb/Exception.h"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 Stream_File::Stream_File
 /////////////////////////////////////////////////////////////////////////////
 (
  std::streambuf &streambuf,
  Open_Mode mode
 ):
  Buffered_File(mode),
  streambuf(streambuf),
  pos(0)
 {
  if (is_shared())
   throw Exception("Stream_File does not support shared_write");

  streambuf.pubseekoff
  (
   pos,
   std::ios_base::beg,
   std::ios_base::in
  );
 }

 /////////////////////////////////////////////////////////////////////////////
 int64_t Stream_File::get_size() const
 /////////////////////////////////////////////////////////////////////////////
 {
  pos = streambuf.pubseekoff
  (
   0,
   std::ios_base::end,
   std::ios_base::in
  );

  return pos;
 }

 /////////////////////////////////////////////////////////////////////////////
 void Stream_File::seek(int64_t offset) const
 /////////////////////////////////////////////////////////////////////////////
 {
  if (int64_t(pos) == offset)
   return;

  if (offset >= 0)
  {
   pos = streambuf.pubseekoff(offset, std::ios_base::beg);
   if (int64_t(pos) == offset)
    return;
  }

  throw Exception("seek error");
 }

 /////////////////////////////////////////////////////////////////////////////
 size_t Stream_File::pread(char *data, size_t size, int64_t offset) const
 /////////////////////////////////////////////////////////////////////////////
 {
  seek(offset);
  const std::streamsize n = streambuf.sgetn(data, std::streamsize(size));
  pos += n;
  return size_t(n);
 }

 /////////////////////////////////////////////////////////////////////////////
 void Stream_File::pwrite(const char *data, size_t size, int64_t offset)
 /////////////////////////////////////////////////////////////////////////////
 {
  seek(offset);

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

   pos += n;
   written += size_t(n);
  }

  if (streambuf.pubsync() < 0)
   throw Exception("sync error");
 }

 /////////////////////////////////////////////////////////////////////////////
 Stream_File::~Stream_File()
 /////////////////////////////////////////////////////////////////////////////
 {
  destructor_flush();
 }
}
