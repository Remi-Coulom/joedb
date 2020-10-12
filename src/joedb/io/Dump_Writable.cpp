#include "joedb/io/Dump_Writable.h"

#include <ctime>

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
std::string Dump_Writable::get_local_time(int64_t timestamp)
/////////////////////////////////////////////////////////////////////////////
{
 const size_t buffer_size = 24;
 char buffer[buffer_size];
 time_t stamp = time_t(timestamp);

 std::strftime
 (
  buffer,
  buffer_size,
  "%Y-%m-%d %H:%M:%S GMT",
  std::gmtime(&stamp)
 );

 return buffer;
}
