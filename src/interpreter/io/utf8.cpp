#include "utf8.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
std::string joedb::read_utf8_string(std::istream &in)
/////////////////////////////////////////////////////////////////////////////
{
 std::string result;
 return result;
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
  const char c = s[i];

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
  else
  {
   out c;
  }
 }

 out << '"';
}
