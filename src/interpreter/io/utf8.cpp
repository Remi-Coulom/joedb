#include "utf8.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
std::string joedb::read_utf8_string(std::istream &in)
/////////////////////////////////////////////////////////////////////////////
{
 std::string result;
 in >> result;
 return result;
}

/////////////////////////////////////////////////////////////////////////////
void write_hexa_character(std::ostream &out, uint8_t c)
/////////////////////////////////////////////////////////////////////////////
{
 out << "\\x" << char('0' + (c >> 4)) << char('0' + (c & 0x0f));
}

/////////////////////////////////////////////////////////////////////////////
void write_utf8_string(std::ostream &out,
                       const std::string &s,
                       bool ascii_only)
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
   else if (c == '\\')
    out << "\\";
   else if (c == '\t')
    out << "\\t";
   else if (c < 0x20)
    write_hexa_character(out, c);
   else
    out << c;
  }
  else if (c < 0xc2)
  {
   write_hexa_character(out, c);
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
   write_hexa_character(out, c);
  }
 }

 out << '"';
}
