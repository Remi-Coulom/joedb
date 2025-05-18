#include "joedb/error/assert.h"

#include <algorithm>
#include <string>
#include <string_view>

namespace joedb
{
 std::string get_error_message
 (
  const char *condition,
  const char *full_file,
  const char *function,
  int line
 )
 {
  std::string cpp_file(full_file);
  std::replace(cpp_file.begin(), cpp_file.end(), '\\', '/');
  size_t pos = cpp_file.find("joedb/src/joedb");
  std::string_view file(cpp_file);
  if (pos != std::string::npos)
   file.remove_prefix(pos + 10);
  return std::string(file) + ":" + std::to_string(line) + ":" + function + ":!(" + condition + ")";
 }
}
