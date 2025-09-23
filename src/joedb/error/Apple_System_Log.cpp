#include "joedb/error/Apple_System_Log.h"

#include <string>

namespace joedb
{
 Apple_System_Log::Apple_System_Log(std::string_view tag):
  log(os_log_create("org.joedb", std::string(tag).c_str())
 {
 }

 void Apple_System_Log::write(std::string_view message) noexcept
 {
  try
  {
   os_log_with_type
   (
    log,
    OS_LOG_TYPE_INFO,
    "%{public}s",
    std::string(message).c_str()
   );
  }
  catch (...)
  {
  }
 }
}
