#include "joedb/error/Apple_Logger.h"

#include <string>

namespace joedb
{
 Apple_Logger::Apple_Logger(const char *tag):
  log(os_log_create("org.joedb", tag))
 {
 }

 void Apple_Logger::write(const std::string &message) noexcept
 {
  os_log_with_type(log, OS_LOG_TYPE_INFO, "%{public}s", message.c_str());
 }

 Apple_Logger::~Apple_Logger()
 {
  os_release(log);
 }
}
