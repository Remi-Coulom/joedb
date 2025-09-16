#ifndef joedb_write_source_code_declared
#define joedb_write_source_code_declared

#include <string>
#include <stddef.h>
#include <functional>

namespace joedb
{
 void write_source_code
 (
  const std::string &dir_name,
  const std::string &file_name,
  const std::function<void(std::ostream &)> &write
 );
}

#endif
