#include "joedb/Generic_File.h"

/////////////////////////////////////////////////////////////////////////////
void joedb::Generic_File::set_position(uint64_t new_position)
/////////////////////////////////////////////////////////////////////////////
{
 flush();
 if (!seek(new_position))
 {
  position = new_position;
  reset_read_buffer();
 }
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Generic_File::write_string(const std::string &s)
/////////////////////////////////////////////////////////////////////////////
{
 compact_write<size_t>(s.size());
 for (char c: s)
  write<char>(c);
}

/////////////////////////////////////////////////////////////////////////////
std::string joedb::Generic_File::read_string()
/////////////////////////////////////////////////////////////////////////////
{
 std::string s;
 size_t size = compact_read<size_t>();
 s.resize(size);
 for (size_t i = 0; i < size; i++)
  s[i] = char(getc());
 return s;
}

/////////////////////////////////////////////////////////////////////////////
std::string joedb::Generic_File::safe_read_string(size_t max_size)
/////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////
void joedb::Generic_File::flush()
/////////////////////////////////////////////////////////////////////////////
{
 if (write_buffer_index)
  flush_write_buffer();
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Generic_File::commit()
/////////////////////////////////////////////////////////////////////////////
{
 flush();
 sync();
}
