#include "joedb/error/assert.h"

#include <cstring>

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
  const char *p = std::strstr(full_file, "joedb/src/joedb");
  const char *file = p ? p + 10 : full_file;
  return std::string(file) + ":" + std::to_string(line) + ":" + std::string(function) + ":!(" + std::string(condition) + ")";
 }
}
