#include "joedb/journal/Generic_File.h"

#include <algorithm>
#include <joedb/Exception.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Generic_File::set_position(int64_t new_position)
 ////////////////////////////////////////////////////////////////////////////
 {
  flush();
  if (!seek(new_position))
  {
   position = new_position;
   reset_read_buffer();
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Generic_File::copy(Generic_File &source)
 ////////////////////////////////////////////////////////////////////////////
 {
  flush();
  source.set_position(0);

  while (true)
  {
   source.read_buffer();
   if (source.read_buffer_size == 0)
    break;
   std::copy_n(source.buffer, source.read_buffer_size, buffer);
   write_buffer_index = source.read_buffer_size;
   write_buffer();
   position += source.read_buffer_size;
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Generic_File::write_string(const std::string &s)
 ////////////////////////////////////////////////////////////////////////////
 {
  compact_write<size_t>(s.size());
  const int64_t blob_position = get_position();
  for (const char c: s)
   write<char>(c);
  return blob_position;
 }

 ////////////////////////////////////////////////////////////////////////////
 std::string Generic_File::read_string()
 ////////////////////////////////////////////////////////////////////////////
 {
  const size_t size = compact_read<size_t>();
  std::string s;
  s.resize(size);
  read_data(&s[0], size);
  return s;
 }

 ////////////////////////////////////////////////////////////////////////////
 Blob Generic_File::read_blob()
 ////////////////////////////////////////////////////////////////////////////
 {
  const size_t size = compact_read<size_t>();
  Blob result(get_position(), size);
  set_position(get_position() + int64_t(size));

  return result;
 }

 ////////////////////////////////////////////////////////////////////////////
 std::string Generic_File::safe_read_string(size_t max_size)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::string s;
  const size_t size = compact_read<size_t>();
  if (size < max_size)
  {
   s.resize(size);
   for (size_t i = 0; i < size; i++)
    s[i] = char(getc());
  }
  return s;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Generic_File::flush()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (write_buffer_index)
   write_buffer();
 }

 ////////////////////////////////////////////////////////////////////////////
 void Generic_File::destructor_flush() noexcept
 ////////////////////////////////////////////////////////////////////////////
 {
  if (write_buffer_index)
  {
   Destructor_Logger::write("warning: an unflushed file is being destroyed");
   try
   {
    flush();
   }
   catch (...)
   {
    postpone_exception("failed to flush in Generic_File destructor");
   }
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Generic_File::commit()
 ////////////////////////////////////////////////////////////////////////////
 {
  flush();
  sync();
 }

 ////////////////////////////////////////////////////////////////////////////
 SHA_256::Hash Generic_File::get_hash
 ////////////////////////////////////////////////////////////////////////////
 (
  const int64_t start,
  const int64_t size
 )
 {
  SHA_256 sha_256;
  const int64_t original_position = get_position();
  set_position(start);

  constexpr uint32_t chunk_size = 64;
  constexpr uint32_t chunks = 2048;
  std::vector<char> hashing_buffer(chunk_size * chunks);

  int64_t current_size = 0;

  while (true)
  {
   size_t requested_size = chunk_size * chunks;
   if (current_size + int64_t(requested_size) > size)
    requested_size = size_t(size - current_size);

   const size_t read_count = raw_read(&hashing_buffer[0], requested_size);
   current_size += read_count;
   const uint32_t full_chunks = uint32_t(read_count / chunk_size);
   for (uint32_t i = 0; i < full_chunks; i++)
    sha_256.process_chunk(&hashing_buffer[i * chunk_size]);

   const uint32_t remainder = uint32_t(read_count % chunk_size);
   if (remainder || current_size >= size || read_count == 0)
   {
    sha_256.process_final_chunk
    (
     &hashing_buffer[full_chunks * chunk_size],
     uint64_t(current_size)
    );
    break;
   }
  }

  set_position(original_position);
  return sha_256.get_hash();
 }

 ////////////////////////////////////////////////////////////////////////////
 SHA_256::Hash Generic_File::get_hash()
 ////////////////////////////////////////////////////////////////////////////
 {
  flush();
  return get_hash(0, get_size());
 }

 ////////////////////////////////////////////////////////////////////////////
 std::string Generic_File::read_blob(Blob blob)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::string result;

  const int64_t current_position = get_position();

  set_position(blob.get_position());
  result.resize(blob.get_size());
  read_data(&result[0], blob.get_size());

  set_position(current_position);

  return result;
 }
}
