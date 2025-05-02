#include "joedb/is_identifier.h"

namespace joedb
{
 constexpr bool is_letter(char c)
 {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
 }

 constexpr bool is_number(char c)
 {
  return ('0' <= c && c <= '9');
 }

 /// @ingroup joedb
 bool is_identifier(const std::string &s)
 {
  if (s.empty())
   return false;

  if (is_number(s[0]))
   return false;

  char previous = 0;
  for (const char c: s)
  {
   if (c != '_' && !is_letter(c) && !is_number(c))
    return false;
   if (c == '_' && previous == '_')
    return false;
   previous = c;
  }

  return true;
 }
}
