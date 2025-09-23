#include "joedb/error/Android_System_Log.h"
#include <android/log.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Android_System_Log::Android_System_Log(std::string_view tag): tag(tag)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Android_System_Log::write(std::string_view message) noexcept
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
