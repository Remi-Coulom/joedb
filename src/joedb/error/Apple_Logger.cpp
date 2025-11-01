#include "joedb/error/Apple_Logger.h"

#include <string>

namespace joedb
{
 Apple_Logger::Apple_Logger(const char *tag):
  os_log(os_log_create("org.joedb", tag))
 {
 }

 void Apple_Logger::log(const std::string &message) noexcept
 {
  os_log_with_type(os_log, OS_LOG_TYPE_INFO, "%{public}s", message.c_str());
 }

 Apple_Logger::~Apple_Logger()
 {
  os_release(os_log);
 }
}
