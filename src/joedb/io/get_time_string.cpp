#include "joedb/io/get_time_string.h"

#include <ctime>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 std::string get_time_string(int64_t timestamp)
 /////////////////////////////////////////////////////////////////////////////
 {
  constexpr size_t buffer_size = 24;
  char buffer[buffer_size];
  const time_t stamp = time_t(timestamp);

  std::strftime
  (
   buffer,
   buffer_size,
   "%Y-%m-%d %H:%M:%S GMT",
   std::gmtime(&stamp)
  );

  return std::string(buffer);
 }
}
