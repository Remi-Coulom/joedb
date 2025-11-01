#include "joedb/error/Posix_Logger.h"

#include <string>

#include <syslog.h>

namespace joedb
{
 Posix_Logger::Posix_Logger(std::string tag): tag(std::move(tag))
 {
 }

 void Posix_Logger::write(const std::string &message) noexcept
 {
  if (tag.empty())
   syslog(LOG_INFO, "%s", message.c_str());
  else
   syslog(LOG_INFO, "%s: %s", tag.c_str(), message.c_str());
 }
}
