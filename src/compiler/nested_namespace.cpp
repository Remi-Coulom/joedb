#include "nested_namespace.h"

#include <sstream>

namespace joedb
{
 static const std::string delimiter("::");

 ////////////////////////////////////////////////////////////////////////////
 std::vector<std::string> split_namespace(const std::string &s)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::vector<std::string> result;

  size_t current = 0;
  size_t next = 0;
  
  while ((next = s.find(delimiter, current)) != std::string::npos)
  {
   result.push_back(s.substr(current, next - current));
   current = next + delimiter.size();
  }

  result.push_back(s.substr(current, s.size() - current));

  return result;
 }

 ////////////////////////////////////////////////////////////////////////////
 std::string namespace_string
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::vector<std::string> &n,
  const char *delimiter
 )
 {
  std::ostringstream result;

  for (size_t i = 0;;)
  {
   result << n[i];
   if (++i < n.size())
    result << delimiter;
   else
    break;
  }

  return result.str();
 }

 ////////////////////////////////////////////////////////////////////////////
 void namespace_open(std::ostream &out, const std::vector<std::string> &n)
 ////////////////////////////////////////////////////////////////////////////
 {
  for (size_t i = 0; i < n.size(); i++)
   out << "namespace " << n[i] << " {";
  out << "\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 void namespace_close(std::ostream &out, const std::vector<std::string> &n)
 ////////////////////////////////////////////////////////////////////////////
 {
  for (size_t i = 0; i < n.size(); i++)
   out << '}';
  out << '\n';
  out << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void namespace_include_guard
 ////////////////////////////////////////////////////////////////////////////
 (
  std::ostream &out,
  const char *name,
  const std::vector<std::string> &n
 )
 {
  out << "#ifndef " << namespace_string(n, "_") << name << '\n';
  out << "#define " << namespace_string(n, "_") << name << '\n';
 }
}
