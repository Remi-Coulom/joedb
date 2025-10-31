#include "joedb/error/Default_System_Logger.h"
#include "joedb/ui/get_time_string.h"

#include <iostream>

namespace joedb
{
 std::mutex Default_System_Logger::mutex;

 ////////////////////////////////////////////////////////////////////////////
 Default_System_Logger::Default_System_Logger(std::string tag): tag(std::move(tag))
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Default_System_Logger::write(const std::string &message) noexcept
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
