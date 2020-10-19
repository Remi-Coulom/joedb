#include "joedb/io/type_io.h"
#include "joedb/Exception.h"

/////////////////////////////////////////////////////////////////////////////
char joedb::get_hex_char_from_digit(uint8_t n)
/////////////////////////////////////////////////////////////////////////////
{
 n &= 0x0f;

 if (n < 10)
  return char('0' + n);
 else
  return char('a' + n - 10);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::write_octal_character(std::ostream &out, uint8_t c)
/////////////////////////////////////////////////////////////////////////////
{
 out.put('\\');
 out.put(char('0' + ((c >> 6) & 7)));
 out.put(char('0' + ((c >> 3) & 7)));
 out.put(char('0' + ((c >> 0) & 7)));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::write_string(std::ostream &out, const std::string &s, bool json)
/////////////////////////////////////////////////////////////////////////////
{
 out.put('"');

 for (size_t i = 0; i < s.size(); i++)
 {
  const uint8_t c = uint8_t(s[i]);

  if (c < 0x20 || c == 0x7f)
  {
   if (json)
   {
    out.put('\\');
    out.put('u');
    out.put('0');
    out.put('0');
    out.put(get_hex_char_from_digit(uint8_t(c >> 4)));
    out.put(get_hex_char_from_digit(uint8_t(c & 0x0f)));
   }
   else
    write_octal_character(out, c);
  }
  else if (c == '"')
   out.put('\\').put('"');
  else if (c == '\\')
   out.put('\\').put('\\');
  else if ((c & 0xe0) == 0xc0 &&
           i + 1 < s.size() &&
           (s[i + 1] & 0xc0) == 0x80)
  {
   out.put(s[i++]);
   out.put(s[i]);
  }
  else if ((c & 0xf0) == 0xe0 &&
           i + 2 < s.size() &&
           (s[i + 1] & 0xc0) == 0x80 &&
           (s[i + 2] & 0xc0) == 0x80)
  {
   out.put(s[i++]);
   out.put(s[i++]);
   out.put(s[i]);
  }
  else if ((c & 0xf8) == 0xf0 &&
           i + 3 < s.size() &&
           (s[i + 1] & 0xc0) == 0x80 &&
           (s[i + 2] & 0xc0) == 0x80 &&
           (s[i + 3] & 0xc0) == 0x80)
  {
   out.put(s[i++]);
   out.put(s[i++]);
   out.put(s[i++]);
   out.put(s[i]);
  }
  else if (c & 0x80)
  {
   if (json)
    throw Exception("json can't handle non-utf8 strings");
   else
    write_octal_character(out, c);
  }
  else
   out.put(char(c));
 }

 out.put('"');
}

/////////////////////////////////////////////////////////////////////////////
void joedb::write_int8(std::ostream &out, int8_t value)
/////////////////////////////////////////////////////////////////////////////
{
 out << int(value);
}
