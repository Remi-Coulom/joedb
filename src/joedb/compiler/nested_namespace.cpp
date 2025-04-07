#include "joedb/compiler/nested_namespace.h"

namespace joedb
{
 static const std::string scope_delimiter{"::"};

 ////////////////////////////////////////////////////////////////////////////
 std::vector<std::string> split_namespace(const std::string &s)
 ////////////////////////////////////////////////////////////////////////////
 {
  std::vector<std::string> result;

  size_t current = 0;
  size_t next = 0;

  while ((next = s.find(scope_delimiter, current)) != std::string::npos)
  {
   result.emplace_back(s.substr(current, next - current));
   current = next + scope_delimiter.size();
  }

  result.emplace_back(s.substr(current, s.size() - current));

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
  out << namespace_string(n, delimiter);
 }

 ////////////////////////////////////////////////////////////////////////////
 std::string namespace_string
 ////////////////////////////////////////////////////////////////////////////
 (
  const std::vector<std::string> &n,
  const char *delimiter
 )
 {
  std::string result;

  for (size_t i = 0;;)
  {
   result += n[i];
   if (++i < n.size())
    result += delimiter;
   else
    break;
  }

  return result;
 }

 ////////////////////////////////////////////////////////////////////////////
 void namespace_open(std::ostream &out, const std::vector<std::string> &n)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "namespace ";
  namespace_write(out, n);
  out << "\n{";
 }

 ////////////////////////////////////////////////////////////////////////////
 void namespace_close(std::ostream &out, const std::vector<std::string> &n)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "}\n";
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
  std::string id = namespace_string(n, "_") + '_' + name + "_declared\n";
  out << "#ifndef " << id;
  out << "#define " << id;
 }
}
