#include "joedb/journal/Generic_File.h"
#include "joedb/Destructor_Logger.h"

#include <algorithm>

namespace joedb
{
 //////////////////////////////////////////////////////////////////////////
 Generic_File::Generic_File(Open_Mode mode):
 //////////////////////////////////////////////////////////////////////////
  mode(mode),
  shared(mode == Open_Mode::shared_write),
  locked_tail
  (
   mode != Open_Mode::shared_write &&
   mode != Open_Mode::read_existing
  )
 {
  read_buffer_size = 0;
  end_of_file = false;
  buffer.index = 0;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Generic_File::flush()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (buffer_has_write_data())
   write_buffer();
  read_buffer_size = 0;
  buffer.index = 0;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Generic_File::set_position(int64_t new_position)
 ////////////////////////////////////////////////////////////////////////////
 {
  flush();
  end_of_file = false;
  seek(new_position);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Generic_File::copy_to
 ////////////////////////////////////////////////////////////////////////////
 (
  Generic_File &destination,
  int64_t start,
  int64_t size
 )
 {
  set_position(start);
  destination.set_position(start);

  while (size > 0)
  {
   read_buffer();
   if (end_of_file)
    break;

   const int64_t copy_size = std::min(size, int64_t(read_buffer_size));
   destination.pos_write(buffer.data, size_t(copy_size));
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
   s.resize(size_t(size));
   read_data(&s[0], size_t(size));
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
}
