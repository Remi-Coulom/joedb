#include "joedb/error/Default_System_Log.h"
#include "joedb/ui/get_time_string.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Default_System_Log::Default_System_Log(std::string_view tag): tag(tag)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Default_System_Log::write(std::string_view message) noexcept
 {
  try
  {
   std::unique_lock lock(mutex);
   std::cerr << joedb::get_time_string_of_now();
   std::cerr << ' ' << tag << ": " << message << '\n';
   std::cerr.flush();
  }
  catch (...)
  {
  }
 }
}
