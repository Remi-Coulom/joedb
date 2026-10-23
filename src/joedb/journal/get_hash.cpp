#include "joedb/journal/get_hash.h"
#include "joedb/journal/Readonly_Memory_File.h"

#include <vector>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 SHA_256::Hash get_full_hash
 ////////////////////////////////////////////////////////////////////////////
 (
  const Abstract_File &file,
  int64_t from,
  int64_t until
 )
 {
  SHA_256 sha_256;

  constexpr uint32_t chunks = 2048;
  std::vector<char> hashing_buffer(SHA_256::chunk_size * chunks);

  int64_t current = from;

  while (true)
  {
   size_t block_size = SHA_256::chunk_size * chunks;
   if (current + int64_t(block_size) > until)
    block_size = size_t(until - current);

   file.full_pread(hashing_buffer.data(), block_size, current);
   current += int64_t(block_size);
   const uint32_t full_chunks = uint32_t(block_size / SHA_256::chunk_size);
   for (uint32_t i = 0; i < full_chunks; i++)
    sha_256.process_chunk(&hashing_buffer[i * SHA_256::chunk_size]);

   const uint32_t remainder = uint32_t(block_size % SHA_256::chunk_size);
   if (remainder || current >= until)
   {
    sha_256.process_final_chunk
    (
     &hashing_buffer[full_chunks * SHA_256::chunk_size],
     uint64_t(current - from)
    );
    break;
   }
  }

  return sha_256.get_hash();
 }

 ////////////////////////////////////////////////////////////////////////////
 SHA_256::Hash get_full_hash(const Abstract_File &file)
 ////////////////////////////////////////////////////////////////////////////
 {
  return get_full_hash(file, 0, file.get_size());
 }

 ////////////////////////////////////////////////////////////////////////////
 SHA_256::Hash get_full_hash(const std::string_view s)
 ////////////////////////////////////////////////////////////////////////////
 {
  Readonly_Memory_File file(s);
  return get_full_hash(file);
 }

 ////////////////////////////////////////////////////////////////////////////
 SHA_256::Hash get_fast_hash
 ////////////////////////////////////////////////////////////////////////////
 (
  const Abstract_File &file,
  int64_t from,
  int64_t until
 )
 {
  std::array<char, 1 << 12> buffer;

  constexpr int buffer_count = 256;
  const int64_t size = until - from;

  if (size < 4 * int64_t(buffer.size()) * buffer_count)
   return get_full_hash(file, from, until);

  SHA_256 sha_256;

  for (int i = 0; i < buffer_count; i++)
  {
   int64_t buffer_position;

   if (i == 0)
    buffer_position = from;
   else if (i == buffer_count - 1)
    buffer_position = until - int64_t(buffer.size());
   else
   {
    buffer_position = int64_t(buffer.size()) *
    (
     (from + i * size) / (int64_t(buffer.size()) * (buffer_count - 1))
    );
   }

   file.full_pread(buffer.data(), buffer.size(), buffer_position);

   for (size_t j = 0; j < buffer.size(); j += SHA_256::chunk_size)
    sha_256.process_chunk(buffer.data() + j);
  }

  return sha_256.get_hash();
 }
}
