#include "joedb/journal/Generic_File.h"
#include "joedb/Destructor_Logger.h"

#include <algorithm>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Generic_File::set_position(int64_t new_position)
 ////////////////////////////////////////////////////////////////////////////
 {
  flush();
  end_of_file = false;
  if (!seek(new_position)) // throw if seek error ???
   file_position = new_position;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Generic_File::copy(Generic_File &source, int64_t start, int64_t size)
 ////////////////////////////////////////////////////////////////////////////
 {
  set_position(start);
  source.set_position(start);

  while (size > 0)
  {
   source.read_buffer();
   if (source.end_of_file)
    break;

   const int64_t copy_size = std::min(size, int64_t(source.read_buffer_size));
   raw_write(source.buffer.data, copy_size); // ??? conversion to size_t ???
   file_position += copy_size;
   size -= copy_size;
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
  const int64_t size = compact_read<int64_t>();
  if (size < max_size)
  {
   s.resize(size);
   read_data(&s[0], size);
  }
  return s;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Generic_File::destructor_flush() noexcept
 ////////////////////////////////////////////////////////////////////////////
 {
  if (buffer_has_write_data())
  {
   Destructor_Logger::write("warning: an unflushed file is being destroyed");
   try
   {
    write_buffer();
   }
   catch (...)
   {
    postpone_exception("failed to flush in Generic_File destructor");
   }
  }
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
  flush();
  seek(start);

  constexpr uint32_t chunks = 2048;
  std::vector<char> hashing_buffer(SHA_256::chunk_size * chunks);

  int64_t current_size = 0;

  while (true)
  {
   size_t requested_size = SHA_256::chunk_size * chunks;
   if (current_size + int64_t(requested_size) > size)
    requested_size = size_t(size - current_size);

   const size_t read_count = read_all(&hashing_buffer[0], requested_size);
   current_size += int64_t(read_count);
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

  seek(file_position);
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

  if (size < 4 * int64_t(buffer.size) * buffer_count)
   return get_hash(start, size);

  SHA_256 sha_256;
  flush();

  for (int i = 0; i < buffer_count; i++)
  {
   int64_t buffer_position;

   if (i == 0)
    buffer_position = start;
   else if (i == buffer_count - 1)
    buffer_position = start + size - buffer.size;
   else
   {
    buffer_position = buffer.size *
    (
     (start + i * size) / (buffer.size * (buffer_count - 1))
    );
   }

   seek(buffer_position);
   read_all(buffer.data, buffer.size);

   for (int j = 0; j < int(buffer.size); j += int(SHA_256::chunk_size))
    sha_256.process_chunk(buffer.data + j);
  }

  seek(file_position);
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
 void Generic_File::shared_lock(int64_t start, int64_t size)
 ////////////////////////////////////////////////////////////////////////////
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Generic_File::exclusive_lock(int64_t start, int64_t size)
 ////////////////////////////////////////////////////////////////////////////
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Generic_File::unlock(int64_t start, int64_t size)
 ////////////////////////////////////////////////////////////////////////////
 {
 }
}
