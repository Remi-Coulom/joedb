#include "joedb/error/Android_System_Logger.h"
#include <android/log.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Android_System_Logger::Android_System_Logger(std::string_view tag): tag(tag)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Android_System_Logger::write(std::string_view message) noexcept
 {
  try
  {
   __android_log_print(ANDROID_LOG_INFO, tag.c_str(), "%s", std::string(message).c_str());
  }
  catch (...)
  {
  }
 }
}
