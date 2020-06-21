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
 void namespace_write
 ////////////////////////////////////////////////////////////////////////////
 (
  std::ostream &out,
  const std::vector<std::string> &n,
  const char *delimiter
 )
 {
  for (size_t i = 0;;)
  {
   out << n[i];
   if (++i < n.size())
    out << delimiter;
   else
    break;
  }
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
  namespace_write(result, n, delimiter);
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
  std::ostringstream id;
  namespace_write(id, n, "_");
  id << '_' << name << "_declared\n";
  out << "#ifndef " << id.str();
  out << "#define " << id.str();
 }
}
