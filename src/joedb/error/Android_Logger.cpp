#include "joedb/error/Android_Logger.h"
#include <android/log.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Android_Logger::Android_Logger(std::string tag): tag(std::move(tag))
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Android_Logger::write(const std::string &message) noexcept
 {
  __android_log_print(ANDROID_LOG_INFO, tag.c_str(), "%s", message.c_str());
 }
}
