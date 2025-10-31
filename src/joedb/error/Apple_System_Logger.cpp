#include "joedb/error/Apple_System_Logger.h"

#include <string>

namespace joedb
{
 Apple_System_Logger::Apple_System_Logger(const char *tag):
  log(os_log_create("org.joedb", tag))
 {
 }

 void Apple_System_Logger::write(const std::string &message) noexcept
 {
  try
  {
   os_log_with_type(log, OS_LOG_TYPE_INFO, "%{public}s", message.c_str());
  }
  catch (...)
  {
  }
 }

 Apple_System_Logger::~Apple_System_Logger()
 {
  os_release(log);
 }
}
