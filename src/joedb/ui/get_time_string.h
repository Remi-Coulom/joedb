#ifndef joedb_get_time_string
#define joedb_get_time_string

#include <stdint.h>
#include <string>

namespace joedb
{
 /// \ingroup ui
 std::string get_time_string(int64_t timestamp);
 /// \ingroup ui
 std::string get_time_string_of_now();
}

#endif
