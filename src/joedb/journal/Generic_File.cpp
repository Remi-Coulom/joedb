#include "joedb/journal/Generic_File.h"

#include <algorithm>

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
 void Generic_File::write_string(const std::string &s)
 ////////////////////////////////////////////////////////////////////////////
 {
  compact_write<size_t>(s.size());
  write_data(&s[0], s.size());
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
 std::string Generic_File::safe_read_string(int64_t max_size)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::string s;
  const size_t size = compact_read<size_t>(); // size_t overflow if 32-bit?
  if (int64_t(size) < max_size)
  {
   s.resize(size);
   for (size_t i = 0; i < size; i++)
    s[i] = char(getc()); // TODO: optimize for large strings
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

  constexpr uint32_t chunks = 2048;
  std::vector<char> hashing_buffer(SHA_256::chunk_size * chunks);

  int64_t current_size = 0;

  while (true)
  {
   size_t requested_size = SHA_256::chunk_size * chunks;
   if (current_size + int64_t(requested_size) > size)
    requested_size = size_t(size - current_size);

   const size_t read_count = raw_read(&hashing_buffer[0], requested_size);
   current_size += read_count;
   const uint32_t full_chunks = uint32_t(read_count / SHA_256::chunk_size);
   for (uint32_t i = 0; i < full_chunks; i++)
    sha_256.process_chunk(&hashing_buffer[i * SHA_256::chunk_size]);

   const uint32_t remainder = uint32_t(read_count % SHA_256::chunk_size);
   if (remainder || current_size >= size || read_count == 0)
   {
    sha_256.process_final_chunk
    (
     &hashing_buffer[full_chunks * SHA_256::chunk_size],
     uint64_t(current_size)
    );
    break;
   }
  }

  set_position(original_position);
  return sha_256.get_hash();
 }

 ////////////////////////////////////////////////////////////////////////////
 SHA_256::Hash Generic_File::get_fast_hash
 ////////////////////////////////////////////////////////////////////////////
 (
  const int64_t start,
  const int64_t size
 )
 {
  constexpr int buffer_count = 256;

  if (size < 4 * buffer_size * buffer_count)
   return get_hash(start, size);

  SHA_256 sha_256;
  const int64_t original_position = get_position();

  for (int i = 0; i < buffer_count; i++)
  {
   int64_t buffer_position;

   if (i == 0)
    buffer_position = start;
   else if (i == buffer_count - 1)
    buffer_position = start + size - buffer_size;
   else
   {
    buffer_position = buffer_size *
    (
     (start + i * size) / (buffer_size * (buffer_count - 1))
    );
   }

   set_position(buffer_position);

   raw_read(buffer, buffer_size);

   for (int j = 0; j < buffer_size; j += SHA_256::chunk_size)
    sha_256.process_chunk(buffer + j);
  }

  reset_read_buffer();
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
 std::string Generic_File::read_blob_data(Blob blob)
 ////////////////////////////////////////////////////////////////////////////
 {
  const int64_t current_position = get_position();
  set_position(blob.get_position());
  std::string result = read_string();
  set_position(current_position);
  return result;
 }

 ////////////////////////////////////////////////////////////////////////////
 Blob Generic_File::write_blob_data(const std::string &data)
 ////////////////////////////////////////////////////////////////////////////
 {
  const int64_t blob_position = get_position();
  write_string(data);
  return Blob(blob_position);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Generic_File::locking_unsupported()
 ////////////////////////////////////////////////////////////////////////////
 {
  throw Exception("Locking unsupported");
 }

 ////////////////////////////////////////////////////////////////////////////
 bool Generic_File::supports_locking() const noexcept
 ////////////////////////////////////////////////////////////////////////////
 {
  return false;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Generic_File::shared_lock()
 ////////////////////////////////////////////////////////////////////////////
 {
  locking_unsupported();
 }

 ////////////////////////////////////////////////////////////////////////////
 void Generic_File::exclusive_lock()
 ////////////////////////////////////////////////////////////////////////////
 {
  locking_unsupported();
 }

 ////////////////////////////////////////////////////////////////////////////
 void Generic_File::unlock()
 ////////////////////////////////////////////////////////////////////////////
 {
  locking_unsupported();
 }
}
