#include "joedb/ui/get_time_string.h"

#include <ctime>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 std::string get_time_string(int64_t timestamp)
 /////////////////////////////////////////////////////////////////////////////
 {
  const time_t stamp = time_t(timestamp);
  struct tm tm;

#ifdef _WIN32
  if (gmtime_s(&tm, &stamp) != 0)
#else
  if (gmtime_r(&stamp, &tm) == nullptr)
#endif
   return "bad timestamp";

  constexpr size_t buffer_size = 64;
  std::string result(buffer_size, '\0');

  const size_t size = std::strftime
  (
   result.data(),
   result.size(),
   "%Y-%m-%d %H:%M:%S GMT",
   &tm
  );

  result.resize(size);
  return result;
 }

 /////////////////////////////////////////////////////////////////////////////
 std::string get_time_string_of_now()
 /////////////////////////////////////////////////////////////////////////////
 {
  return get_time_string(std::time(nullptr));
 }
}
