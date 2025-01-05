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
  Generic_File(mode),
  streambuf(streambuf)
 {
  if (is_shared())
   throw Exception("Stream_File does not support shared_write");
 }

 /////////////////////////////////////////////////////////////////////////////
 int64_t Stream_File::get_size() const
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
  return size_t(streambuf.sgetn(buffer, std::streamsize(size)));
 }

 /////////////////////////////////////////////////////////////////////////////
 void Stream_File::raw_write(const char *buffer, size_t size)
 /////////////////////////////////////////////////////////////////////////////
 {
  size_t written = 0;

  while (written < size)
  {
   const auto result = streambuf.sputn
   (
    buffer + written,
    std::streamsize(size - written)
   );
   if (result <= 0)
    throw Exception("Could not write to stream");
   written += size_t(result);
  }

  if (streambuf.pubsync() < 0)
   throw Exception("sync error");
 }

 /////////////////////////////////////////////////////////////////////////////
 void Stream_File::raw_seek(int64_t offset)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (offset < 0 || streambuf.pubseekpos(offset) != offset)
   throw Exception("seek error");
 }

 /////////////////////////////////////////////////////////////////////////////
 Stream_File::~Stream_File()
 /////////////////////////////////////////////////////////////////////////////
 {
  destructor_flush();
 }
}
