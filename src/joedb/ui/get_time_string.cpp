#include "joedb/ui/get_time_string.h"

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
  struct tm tm;
#if defined(__unix__)
  gmtime_r(&stamp, &tm);
#else
  tm = *std::gmtime(&stamp);
#endif
  std::strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S GMT", &tm);

  return std::string(buffer);
 }

 /////////////////////////////////////////////////////////////////////////////
 std::string get_time_string_of_now()
 /////////////////////////////////////////////////////////////////////////////
 {
  return get_time_string(std::time(nullptr));
 }
}
