#include "type_io.h"
#include "Markus_Kuhn_wcwidth.h"

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

 const bool is_quoted = c == '"';
 if (is_quoted)
  in.get(c);

 while ((is_quoted && c != '"') || (!is_quoted && c != ' '))
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
   else if (c >= '0' && c <= '9')
   {
    char c1, c0;
    in.get(c1).get(c0);
    const uint8_t n2 = uint8_t(c  - '0');
    const uint8_t n1 = uint8_t(c1 - '0');
    const uint8_t n0 = uint8_t(c0 - '0');
    c = char((n2 << 6) | (n1 << 3) | n0);
   }
  }

  result.push_back(c);

  if (!in.get(c))
   break;
 }

 return result;
}

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
void joedb::write_octal_character(std::ostream &out, uint8_t c)
/////////////////////////////////////////////////////////////////////////////
{
 out.put('\\');
 out.put(char('0' + ((c >> 6) & 7)));
 out.put(char('0' + ((c >> 3) & 7)));
 out.put(char('0' + ((c >> 0) & 7)));
}

/////////////////////////////////////////////////////////////////////////////
void joedb::write_sql_string(std::ostream &out, const std::string &s)
/////////////////////////////////////////////////////////////////////////////
{
 out.put('X');
 out.put('\'');
 for (char c: s)
 {
  out.put(get_hex_char_from_digit(uint8_t(c >> 4)));
  out.put(get_hex_char_from_digit(uint8_t(c & 0x0f)));
 }
 out.put('\'');
}

/////////////////////////////////////////////////////////////////////////////
size_t joedb::utf8_display_size(const std::string &s)
/////////////////////////////////////////////////////////////////////////////
{
 std::setlocale(LC_ALL, "en_US.utf8");
 size_t result = 0;

 std::mbstate_t state = std::mbstate_t();
 const char *current = s.c_str();
 int remaining = int(s.size());

 while (remaining > 0)
 {
  wchar_t wide_char;
  std::size_t n = std::mbrtowc(&wide_char, current, size_t(remaining), &state);
  current += n;
  remaining -= int(n);
  const int w = ::Markus_Kuhn_wcwidth(wide_char);
  if (w > 0)
   result += size_t(::Markus_Kuhn_wcwidth(wide_char));
 }

 return result;
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
   write_octal_character(out, c);
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
   write_octal_character(out, c);
  else
   out.put(char(c));
 }

 out.put('"');
}

/////////////////////////////////////////////////////////////////////////////
int8_t joedb::read_int8(std::istream &in)
/////////////////////////////////////////////////////////////////////////////
{
 int result;
 in >> result;
 return int8_t(result);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::write_int8(std::ostream &out, int8_t value)
/////////////////////////////////////////////////////////////////////////////
{
 out << int(value);
}
