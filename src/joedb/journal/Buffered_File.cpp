#include "joedb/journal/Buffered_File.h"
#include "joedb/journal/Async_Reader.h"
#include "joedb/error/Destructor_Logger.h"

#include <algorithm>

namespace joedb
{
 //////////////////////////////////////////////////////////////////////////
 Buffered_File::Buffered_File(Open_Mode mode):
 //////////////////////////////////////////////////////////////////////////
  mode(mode),
  locked_tail
  (
   mode != Open_Mode::shared_write &&
   mode != Open_Mode::read_existing
  )
 {
  read_buffer_size = 0;
  buffer.index = 0;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Buffered_File::flush()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (buffer_has_write_data())
   write_buffer();
  read_buffer_size = 0;
  buffer.index = 0;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Buffered_File::flush_for_writing()
 ////////////////////////////////////////////////////////////////////////////
 {
  set_position(get_position());
 }

 ////////////////////////////////////////////////////////////////////////////
 void Buffered_File::set_position(int64_t new_position)
 ////////////////////////////////////////////////////////////////////////////
 {
  flush();
  sequential_seek(new_position);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Buffered_File::copy_to
 ////////////////////////////////////////////////////////////////////////////
 (
  Buffered_File &destination,
  const int64_t start,
  const int64_t size
 ) const
 {
  int64_t done = 0;

  while (done < size)
  {
   const size_t asked = size_t(std::min(int64_t(destination.buffer.size), size - done));
   const size_t received = pread(destination.buffer.data, asked, start + done);

   if (received == 0)
    reading_past_end_of_file();

   destination.pwrite(destination.buffer.data, received, start + done);
   done += int64_t(received);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 bool Buffered_File::equal_to
 ////////////////////////////////////////////////////////////////////////////
 (
  Buffered_File &destination,
  const int64_t from,
  const int64_t until
 ) const
 {
  destination.flush();

  for (int64_t current = from; current < until;)
  {
   const int64_t half_buffer_size = Buffered_File::buffer.ssize / 2;

   const size_t n0 = pread
   (
    destination.buffer.data,
    size_t(std::min(half_buffer_size, until - current)),
    current
   );

   size_t n1 = 0;

   while (n1 < n0)
   {
    const size_t n = destination.pread
    (
     destination.buffer.data + half_buffer_size + n1,
     n0 - n1,
     current
    );

    if (n == 0)
     break;

    n1 += n;
   }

   if (n1 != n0)
    return false;

   if (n0 == 0)
    reading_past_end_of_file();

   const int diff = std::memcmp
   (
    destination.buffer.data,
    destination.buffer.data + half_buffer_size,
    n0
   );

   if (diff)
    return false;

   current += int64_t(n0);
  }

  return true;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Buffered_File::write_string(const std::string &s)
 ////////////////////////////////////////////////////////////////////////////
 {
  compact_write<size_t>(s.size());
  write_data(s.data(), s.size());
 }

 ////////////////////////////////////////////////////////////////////////////
 std::string Buffered_File::read_string()
 ////////////////////////////////////////////////////////////////////////////
 {
  const size_t size = compact_read<size_t>();
  std::string s(size, 0);
  read_data(s.data(), size);
  return s;
 }

 ////////////////////////////////////////////////////////////////////////////
 std::string Buffered_File::safe_read_string(int64_t max_size)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::string s;
  const int64_t size = compact_read<int64_t>();
  if (size > 0 && size < max_size)
  {
   s.resize(size_t(size));
   read_data(s.data(), size_t(size));
  }
  return s;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Buffered_File::destructor_flush() noexcept
 ////////////////////////////////////////////////////////////////////////////
 {
  if (buffer_has_write_data())
  {
   Destructor_Logger::write("warning: an unflushed file is being destroyed");
   try { write_buffer(); } catch (...) {}
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 std::string Buffered_File::read_blob(Blob blob) const
 ////////////////////////////////////////////////////////////////////////////
 {
  Async_Reader reader(*this, blob.get_position(), blob.get_end());
  std::string result(size_t(blob.get_size()), 0);
  reader.read(result.data(), result.size());
  if (reader.is_end_of_file())
   reading_past_end_of_file();
  return result;
 }
}
