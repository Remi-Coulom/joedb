#include "joedb/error/CLog_Logger.h"
#include "joedb/ui/get_time_string.h"

#include <iostream>

namespace joedb
{
 std::mutex CLog_Logger::mutex;

 ////////////////////////////////////////////////////////////////////////////
 CLog_Logger::CLog_Logger(std::string tag): tag(std::move(tag))
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void CLog_Logger::log(const std::string &message) noexcept
 {
  try
  {
   std::unique_lock lock(mutex);
   std::clog << joedb::get_time_string_of_now() << ' ';
   if (!tag.empty())
    std::clog << tag << ": ";
   std::clog << message << '\n';
   std::clog.flush();
  }
  catch (...)
  {
  }
 }
}
