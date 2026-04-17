#ifndef joedb_base64_declared
#define joedb_base64_declared

#include <string>

namespace joedb
{
 /// @ingroup ui
 std::string base64_encode(std::string_view input);
 /// @ingroup ui
 std::string base64_decode(std::string_view input);
}

#endif
