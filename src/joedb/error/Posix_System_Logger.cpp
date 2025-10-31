#include "joedb/error/Posix_System_Logger.h"

#include <string>

#include <syslog.h>

namespace joedb
{
 Posix_System_Logger::Posix_System_Logger(std::string_view tag)
 {
  static std::string tag_storage;
  tag_storage = std::string(tag);
  openlog(tag_storage.c_str(), LOG_PID, LOG_USER);
 }

 void Posix_System_Logger::write(const std::string &message) noexcept
 {
  syslog(LOG_INFO, "%s", message.c_str());
 }
}
