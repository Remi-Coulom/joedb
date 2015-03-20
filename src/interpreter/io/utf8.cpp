#include "utf8.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
std::string joedb::read_utf8_string(std::istream &in)
/////////////////////////////////////////////////////////////////////////////
{
 std::string result;

 char c;
 in >> c;

 if (c != '"')
  in >> result;
 else
 {
  while ((in >> c) && c != '"')
  {
   if (c == '\\')
   {
    if (in >> c)
    {
     if (c == 'n')
      c = '\n';
     else if (c == 't')
      c = '\t';
     else if (c == 'x')
     {
      char c1, c0;
      in >> c1 >> c0;
      const uint8_t n1 = get_hex_digit_from_char(c1);
      const uint8_t n0 = get_hex_digit_from_char(c0);
      c = char((n1 << 4) | n0);
     }
    }
   }

   result.push_back(c);
  }
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

/////////////////////////////////////////////////////////////////////////////
void joedb::write_hexa_character(std::ostream &out, uint8_t c)
/////////////////////////////////////////////////////////////////////////////
{
 out << "\\x" << get_hex_char_from_digit(c >> 4) <<
                 get_hex_char_from_digit(c & 0x0f);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::write_utf8_string(std::ostream &out, const std::string &s)
/////////////////////////////////////////////////////////////////////////////
{
 out << '"';

 for (size_t i = 0; i < s.size(); i++)
 {
  const uint8_t c = uint8_t(s[i]);

  if (c < 0x80)
  {
   if (c == '"')
    out << "\\\"";
   else if (c == '\\')
    out << "\\";
   else if (c == '\n')
    out << "\\n";
   else if (c == '\t')
    out << "\\t";
   else if (c < 0x20)
    write_hexa_character(out, c);
   else
    out << c;
  }
  else if (c < 0xc2)
  {
   write_hexa_character(out, c); // error
  }
  else if (c < 0xe0 && i < s.size() - 1)
  {
   out << s[i];
   out << s[++i];
  }
  else if (c < 0xf0 && i < s.size() - 2)
  {
   out << s[i];
   out << s[++i];
   out << s[++i];
  }
  else if (c < 0xf5 && i < s.size() - 3)
  {
   out << s[i];
   out << s[++i];
   out << s[++i];
   out << s[++i];
  }
  else
  {
   write_hexa_character(out, c); // error
  }
 }

 out << '"';
}
