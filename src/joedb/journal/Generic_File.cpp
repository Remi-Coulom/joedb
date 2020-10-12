#include "joedb/journal/Generic_File.h"

/////////////////////////////////////////////////////////////////////////////
void joedb::Generic_File::set_position(int64_t new_position)
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
std::vector<char> joedb::Generic_File::read_tail(int64_t starting_position)
/////////////////////////////////////////////////////////////////////////////
{
 const int64_t initial_position = get_position();

 set_position(starting_position);
 std::vector<char> result;

 while (true)
 {
  const size_t n = read_buffer();
  if (n == 0)
   break;
  result.insert(result.end(), buffer, buffer + n);
 }

 set_position(initial_position);

 return result;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Generic_File::append_tail(const std::vector<char> &data)
/////////////////////////////////////////////////////////////////////////////
{
 const char *current = &data[0];
 size_t remaining = data.size();

 while (remaining)
 {
  const size_t n = remaining > buffer_size ? buffer_size : remaining;
  std::copy(current, current + n, buffer); // TODO: avoid useless copy
  write_buffer_index = n;
  flush_write_buffer();
  current += n;
  remaining -= n;
  position += n;
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
