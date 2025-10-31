#include "joedb/error/Android_System_Logger.h"
#include <android/log.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Android_System_Logger::Android_System_Logger(std::string tag): tag(std::move(tag))
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Android_System_Logger::write(const std::string &message) noexcept
 {
  try
  {
   __android_log_print(ANDROID_LOG_INFO, tag.c_str(), "%s", message.c_str());
  }
  catch (...)
  {
  }
 }
}
