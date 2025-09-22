#include "joedb/error/Default_System_Logger.h"
#include "joedb/ui/get_time_string.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Default_System_Logger::Default_System_Logger(std::string_view tag): tag(tag)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Default_System_Logger::write(std::string_view message) noexcept
 {
  try
  {
   std::cerr << joedb::get_time_string_of_now();
   std::cerr << ' ' << tag << ": " << message << '\n';
   std::cerr.flush();
  }
  catch (...)
  {
  }
 }
}
