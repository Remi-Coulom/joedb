#include "type_io.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
std::string joedb::read_string(std::istream &in)
/////////////////////////////////////////////////////////////////////////////
{
 std::string result;

 char c;
 while (in.get(c) && c == ' ')
 {
 }

 while ((in.get(c)) && c != '"')
 {
  if (c == '\\')
  {
   in.get(c);

   if (c == 'x')
   {
    char c1, c0;
    in.get(c1).get(c0);
    const uint8_t n1 = get_hex_digit_from_char(c1);
    const uint8_t n0 = get_hex_digit_from_char(c0);
    c = char((n1 << 4) | n0);
   }
  }

  result.push_back(c);
 }

 return result;
}

/////////////////////////////////////////////////////////////////////////////
char joedb::get_hex_char_from_digit(uint8_t n)
/////////////////////////////////////////////////////////////////////////////
{
 if (n < 10)
  return char('0' + n);
 else
  return char('a' + n - 10);
}

/////////////////////////////////////////////////////////////////////////////
uint8_t joedb::get_hex_digit_from_char(char c)
/////////////////////////////////////////////////////////////////////////////
{
 if (c >= '0' && c <= '9')
  return uint8_t(c - '0');
 else if (c >= 'a' && c <= 'f')
  return uint8_t(10 + c - 'a');
 else
  return 0;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::write_hexa_character(std::ostream &out, uint8_t c)
/////////////////////////////////////////////////////////////////////////////
{
 out.put('\\');
 out.put('x');
 out.put(get_hex_char_from_digit(uint8_t(c >> 4)));
 out.put(get_hex_char_from_digit(uint8_t(c & 0x0f)));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::write_string(std::ostream &out, const std::string &s)
/////////////////////////////////////////////////////////////////////////////
{
 out.put('"');

 for (size_t i = 0; i < s.size(); i++)
 {
  const uint8_t c = uint8_t(s[i]);

  if (c < 0x20)
   write_hexa_character(out, c);
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
   write_hexa_character(out, c);
  else
   out.put(char(c));
 }

 out.put('"');
}
