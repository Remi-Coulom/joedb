#include "joedb/journal/filebuf.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 filebuf::pos_type filebuf::seekpos
 ////////////////////////////////////////////////////////////////////////////
 (
  pos_type pos,
  std::ios_base::openmode which
 )
 {
  if (which & std::ios_base::in)
   get_position = pos;
  if (which & std::ios_base::out)
   put_position = pos;
  return pos;
 }

 ////////////////////////////////////////////////////////////////////////////
 filebuf::int_type filebuf::underflow()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (file.buffer_has_write_data())
   file.flush();

  if (file.get_position() != get_position)
   file.set_position(get_position);

  size_t available = file.get_available_bytes();

  if (available == 0)
  {
   file.read_buffer_without_throwing();
   available = file.get_available_bytes();
  }

  if (available)
  {
   char *start = file.buffer.data + file.buffer.index;
   char *end = file.buffer.data + file.read_buffer_size;
   setg(start, start, end);

   get_position += available;
   file.buffer.index += available;

   return *start;
  }
  else
   return Traits::eof();
 }

 ////////////////////////////////////////////////////////////////////////////
 std::streamsize filebuf::xsputn(const char* s, std::streamsize count)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (file.get_position() != put_position)
   file.set_position(put_position);
  file.write_data(s, count);
  put_position += count;
  return count;
 }

 ////////////////////////////////////////////////////////////////////////////
 filebuf::int_type filebuf::overflow(filebuf::int_type ch)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (file.get_position() != put_position)
   file.set_position(put_position);

  if (ch == Traits::eof())
  {
   file.flush();
   return 0;
  }
  else
  {
   file.write<char>(char(ch));
   put_position++;
   return 1;
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 int filebuf::sync()
 ////////////////////////////////////////////////////////////////////////////
 {
  file.flush();
  return 0;
 }
}
