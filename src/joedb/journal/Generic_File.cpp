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
 void Generic_File::copy(Generic_File &file)
 ////////////////////////////////////////////////////////////////////////////
 {
  flush();
  file.flush();
  file.set_position(0);

  while (true)
  {
   file.read_buffer();
   if (file.read_buffer_size == 0)
    break;
   std::copy_n(file.buffer, file.read_buffer_size, buffer);
   write_buffer_index = file.read_buffer_size;
   write_buffer();
   position += file.read_buffer_size;
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Generic_File::write_string(const std::string &s)
 ////////////////////////////////////////////////////////////////////////////
 {
  compact_write<size_t>(s.size());
  for (char c: s)
   write<char>(c);
 }

 ////////////////////////////////////////////////////////////////////////////
 std::string Generic_File::read_string()
 ////////////////////////////////////////////////////////////////////////////
 {
  std::string s;
  size_t size = compact_read<size_t>();
  s.resize(size);
  read_data(&s[0], size);
  return s;
 }

 ////////////////////////////////////////////////////////////////////////////
 std::string Generic_File::safe_read_string(size_t max_size)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::string s;
  size_t size = compact_read<size_t>();
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

  const uint32_t chunk_size = 64;
  const uint32_t chunks = 2048;
  std::vector<char> buffer(chunk_size * chunks);

  int64_t current_size = 0;

  while (true)
  {
   size_t requested_size = chunk_size * chunks;
   if (current_size + int64_t(requested_size) > size)
    requested_size = size_t(size - current_size);

   const size_t read_count = raw_read(&buffer[0], requested_size);
   current_size += read_count;
   const uint32_t full_chunks = uint32_t(read_count / chunk_size);
   for (uint32_t i = 0; i < full_chunks; i++)
    sha_256.process_chunk(&buffer[i * chunk_size]);

   const uint32_t remainder = uint32_t(read_count % chunk_size);
   if (remainder || current_size == size)
   {
    sha_256.process_final_chunk
    (
     &buffer[full_chunks * chunk_size],
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
  return get_hash(0, get_size());
 }
}
