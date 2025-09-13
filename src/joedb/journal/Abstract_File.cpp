#include "joedb/journal/Abstract_File.h"
#include "joedb/journal/Async_Reader.h"
#include "joedb/journal/Buffer.h"
#include "joedb/error/Exception.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Abstract_File::copy_to
 ////////////////////////////////////////////////////////////////////////////
 (
  Abstract_File &destination,
  const int64_t start,
  const int64_t size
 ) const
 {
  Buffer<12> buffer;

  int64_t done = 0;

  while (done < size)
  {
   const size_t asked = size_t(std::min(int64_t(buffer.size), size - done));
   const size_t received = pread(buffer.data, asked, start + done);

   if (received == 0)
    reading_past_end_of_file();

   destination.pwrite(buffer.data, received, start + done);
   done += int64_t(received);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 bool Abstract_File::equal_to
 ////////////////////////////////////////////////////////////////////////////
 (
  const Abstract_File &destination,
  const int64_t from,
  const int64_t until
 ) const
 {
  Buffer<12> buffer;
  Buffer<12> destination_buffer;

  for (int64_t current = from; current < until;)
  {
   const size_t n0 = pread
   (
    buffer.data,
    size_t(std::min(buffer.ssize, until - current)),
    current
   );

   size_t n1 = 0;

   while (n1 < n0)
   {
    const size_t n = destination.pread
    (
     destination_buffer.data + n1,
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
    buffer.data,
    destination_buffer.data,
    n0
   );

   if (diff)
    return false;

   current += int64_t(n0);
  }

  return true;
 }

 //////////////////////////////////////////////////////////////////////////
 void Abstract_File::reading_past_end_of_file()
 //////////////////////////////////////////////////////////////////////////
 {
  throw Exception("Trying to read past the end of file");
 }

 ////////////////////////////////////////////////////////////////////////////
 std::string Abstract_File::read_blob(Blob blob) const
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
