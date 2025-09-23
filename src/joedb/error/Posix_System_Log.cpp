#include "joedb/error/Posix_System_Log.h"

#include <string>

namespace joedb
{
 Posix_System_Log::Posix_System_Log(std::string_view tag)
 {
  static std::string tag_storage;
  tag_storage = std::string(tag);
  openlog(tag_storage.c_str(), LOG_PID, LOG_USER);
 }

 void Posix_System_Log::write(std::string_view message) noexcept
 {
  try
  {
   syslog(LOG_INFO, "%s", std::string(message).c_str());
  }
  catch (...)
  {
  }
 }
}
