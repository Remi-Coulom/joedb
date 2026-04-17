#include "joedb/error/Stream_Logger.h"
#include "joedb/ui/get_time_string.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Stream_Logger::Stream_Logger(std::ostream &out, std::string tag):
  out(out),
  tag(std::move(tag))
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Stream_Logger::log(beman::cstring_view message) noexcept
 {
  try
  {
   std::unique_lock lock(mutex);
   out << joedb::get_time_string_of_now() << ' ';
   if (!tag.empty())
    out << tag << ": ";
   out << message << '\n';
   out.flush();
  }
  catch (...)
  {
  }
 }
}
