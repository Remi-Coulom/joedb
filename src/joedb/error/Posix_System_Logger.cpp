#include "joedb/error/Posix_System_Logger.h"

#include <string>

#include <syslog.h>

namespace joedb
{
 Posix_System_Logger::Posix_System_Logger(std::string tag): tag(std::move(tag))
 {
 }

 void Posix_System_Logger::write(const std::string &message) noexcept
 {
  syslog(LOG_INFO, "%s: %s", tag.c_str(), message.c_str());
 }
}
