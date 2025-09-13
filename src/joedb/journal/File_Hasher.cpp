#include "joedb/journal/File_Hasher.h"
#include "joedb/journal/Readonly_Journal.h"
#include "joedb/journal/Readonly_Memory_File.h"

#include <vector>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 SHA_256::Hash File_Hasher::get_hash
 ////////////////////////////////////////////////////////////////////////////
 (
  const Abstract_File &file,
  int64_t start,
  int64_t size
 )
 {
  SHA_256 sha_256;

  constexpr uint32_t chunks = 2048;
  std::vector<char> hashing_buffer(SHA_256::chunk_size * chunks);

  int64_t current_size = 0;

  while (true)
  {
   size_t block_size = SHA_256::chunk_size * chunks;
   if (current_size + int64_t(block_size) > size)
    block_size = size_t(size - current_size);

   const size_t read_count = file.pread(hashing_buffer.data(), block_size, start + current_size);
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

  return sha_256.get_hash();
 }

 ////////////////////////////////////////////////////////////////////////////
 SHA_256::Hash File_Hasher::get_hash(const Abstract_File &file)
 ////////////////////////////////////////////////////////////////////////////
 {
  return File_Hasher::get_hash(file, 0, file.get_size());
 }

 ////////////////////////////////////////////////////////////////////////////
 SHA_256::Hash File_Hasher::get_hash(const std::string &s)
 ////////////////////////////////////////////////////////////////////////////
 {
  Readonly_Memory_File file(s.data(), s.size());
  return File_Hasher::get_hash(file);
 }

 ////////////////////////////////////////////////////////////////////////////
 SHA_256::Hash File_Hasher::get_fast_hash
 ////////////////////////////////////////////////////////////////////////////
 (
  const Abstract_File &file,
  int64_t start,
  int64_t size
 )
 {
  Buffer<12> buffer;

  constexpr int buffer_count = 256;

  if (size < 4 * buffer.ssize * buffer_count)
   return get_hash(file, start, size);

  SHA_256 sha_256;

  for (int i = 0; i < buffer_count; i++)
  {
   int64_t buffer_position;

   if (i == 0)
    buffer_position = start;
   else if (i == buffer_count - 1)
    buffer_position = start + size - buffer.ssize;
   else
   {
    buffer_position = buffer.ssize *
    (
     (start + i * size) / (buffer.ssize * (buffer_count - 1))
    );
   }

   file.pread(buffer.data, buffer.size, buffer_position);

   for (size_t j = 0; j < buffer.size; j += SHA_256::chunk_size)
    sha_256.process_chunk(buffer.data + j);
  }

  return sha_256.get_hash();
 }

 ////////////////////////////////////////////////////////////////////////////
 SHA_256::Hash Journal_Hasher::get_fast_hash
 ////////////////////////////////////////////////////////////////////////////
 (
  const Readonly_Journal &journal,
  int64_t checkpoint
 )
 {
  return File_Hasher::get_fast_hash
  (
   journal.abstract_file,
   Header::size,
   checkpoint - Header::ssize
  );
 }

 ////////////////////////////////////////////////////////////////////////////
 SHA_256::Hash Journal_Hasher::get_full_hash
 ////////////////////////////////////////////////////////////////////////////
 (
  const Readonly_Journal &journal,
  int64_t checkpoint
 )
 {
  return File_Hasher::get_hash
  (
   journal.abstract_file,
   Header::size,
   checkpoint - Header::ssize
  );
 }
}
