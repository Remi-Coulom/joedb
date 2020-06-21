#ifndef joedb_nested_namespace_declared
#define joedb_nested_namespace_declared

#include <vector>
#include <string>
#include <iostream>

namespace joedb
{
 std::vector<std::string> split_namespace(const std::string &s);

 std::string namespace_string
 (
  const std::vector<std::string> &n,
  const char *delimiter = "::"
 );

 void namespace_open(std::ostream &out, const std::vector<std::string> &n);
 void namespace_close(std::ostream &out, const std::vector<std::string> &n);

 void namespace_write
 (
  std::ostream &out,
  const std::vector<std::string> &n,
  const char *delimiter
 );

 void namespace_include_guard
 (
  std::ostream &out,
  const char *name,
  const std::vector<std::string> &n
 );
}

#endif
