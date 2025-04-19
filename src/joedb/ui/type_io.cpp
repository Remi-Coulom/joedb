#include "joedb/ui/type_io.h"
#include "external/wide_char_display_width.h"

#include <iostream>
#include <string>

#include "joedb/ui/minimal_runtime_io.cpp"

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 std::string read_string(std::istream &in)
 /////////////////////////////////////////////////////////////////////////////
 {
  std::string result;

  char c;

  while (true)
  {
   if (!in.get(c))
    return result;
   if (c != ' ')
    break;
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
     char c1;
     char c0;
     in.get(c1).get(c0);
     const uint8_t n1 = get_hex_digit_from_char(c1);
     const uint8_t n0 = get_hex_digit_from_char(c0);
     c = char((n1 << 4) | n0);
    }
    else if (c >= '0' && c <= '9')
    {
     char c1;
     char c0;
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
 uint8_t get_hex_digit_from_char(char c)
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
 void write_hexa_character(std::ostream &out, uint8_t c)
 /////////////////////////////////////////////////////////////////////////////
 {
  out.put('\\');
  out.put('x');
  out.put(get_hex_char_from_digit(uint8_t(c >> 4)));
  out.put(get_hex_char_from_digit(uint8_t(c & 0x0f)));
 }

 /////////////////////////////////////////////////////////////////////////////
 void write_sql_string(std::ostream &out, const std::string &s)
 /////////////////////////////////////////////////////////////////////////////
 {
  out.put('X');
  out.put('\'');
  for (const char c: s)
  {
   out.put(get_hex_char_from_digit(uint8_t(c >> 4)));
   out.put(get_hex_char_from_digit(uint8_t(c & 0x0f)));
  }
  out.put('\'');
 }

 /////////////////////////////////////////////////////////////////////////////
 size_t utf8_display_size(const std::string &s)
 /////////////////////////////////////////////////////////////////////////////
 {
  size_t result = 0;

  for (size_t i = 0; i < s.size();)
  {
   const uint32_t wide_char = read_utf8_char(i, s);
   const int width = wide_char_display_width(uint32_t(wide_char));
   if (width > 0)
    result += size_t(wide_char_display_width(uint32_t(wide_char)));
  }

  return result;
 }

 /////////////////////////////////////////////////////////////////////////////
 uint32_t read_utf8_char(size_t &i, const std::string &s)
 /////////////////////////////////////////////////////////////////////////////
 {
  const uint8_t *p = (reinterpret_cast<const uint8_t *>(s.c_str()) + i);

  uint32_t result;

  if ((p[0] & 0xe0) == 0xc0 && i + 1 < s.size())
  {
   result = (uint32_t(p[0]) << 6) +
            (uint32_t(p[1])     ) - uint32_t(0x3080UL);
   i += 2;
  }
  else if ((p[0] & 0xf0) == 0xe0 && i + 2 < s.size())
  {
   result = (uint32_t(p[0]) << 12) +
            (uint32_t(p[1]) <<  6) +
            (uint32_t(p[2])      ) - uint32_t(0xe2080UL);
   i += 3;
  }
  else if ((p[0] & 0xf8) == 0xf0 && i + 3 < s.size())
  {
   result = (uint32_t(p[0]) << 18) +
            (uint32_t(p[1]) << 12) +
            (uint32_t(p[2]) <<  6) +
            (uint32_t(p[3])      ) - uint32_t(0x3c82080UL);
   i += 4;
  }
  else
  {
   result = p[0];
   i += 1;
  }

  return result;
 }

 /////////////////////////////////////////////////////////////////////////////
 void write_justified
 /////////////////////////////////////////////////////////////////////////////
 (
  std::ostream &out,
  const std::string &s,
  size_t width,
  bool flush_left
 )
 {
  size_t length = 0;
  std::string displayed;

  for (size_t i = 0; s.c_str()[i];)
  {
   const size_t previous_i = i;
   const uint32_t wide_char = read_utf8_char(i, s);
   const size_t char_width = size_t(wide_char_display_width(wide_char));

   if (length + char_width < width ||
       (length + char_width == width && s.c_str()[i] == 0))
   {
    length += char_width;
    for (size_t j = previous_i; j < i; j++)
     displayed += s[j];
   }
   else
   {
    length += 1;
    displayed += "…";
    break;
   }
  }

  if (flush_left)
   out << displayed;

  while (length < width)
  {
   out << ' ';
   length++;
  }

  if (!flush_left)
   out << displayed;
 }

 /////////////////////////////////////////////////////////////////////////////
 int8_t read_int8(std::istream &in)
 /////////////////////////////////////////////////////////////////////////////
 {
  int result;
  in >> result;
  return int8_t(result);
 }

 /////////////////////////////////////////////////////////////////////////////
 bool read_boolean(std::istream &in)
 /////////////////////////////////////////////////////////////////////////////
 {
  std::string word;
  in >> word;

  if (word == "true" || word == "1")
   return true;
  else if (word == "false" || word == "0")
   return false;
  else
  {
   in.setstate(std::ios::failbit);
   return false;
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 void write_boolean(std::ostream &out, bool value)
 /////////////////////////////////////////////////////////////////////////////
 {
  if (value)
   out << "true";
  else
   out << "false";
 }

 /////////////////////////////////////////////////////////////////////////////
 void write_blob(std::ostream &out, Blob blob)
 /////////////////////////////////////////////////////////////////////////////
 {
  out << blob.get_position() << ',' << blob.get_size();
 }

 /////////////////////////////////////////////////////////////////////////////
 Blob read_blob(std::istream &in)
 /////////////////////////////////////////////////////////////////////////////
 {
  int64_t position = 0;
  int64_t size = 0;
  in >> position;
  in.ignore(1, ',');
  in >> size;
  return Blob(position, size);
 }
}
