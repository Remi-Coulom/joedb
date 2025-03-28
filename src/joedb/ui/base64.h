#ifndef joedb_base64_declared
#define joedb_base64_declared

#include <string>

namespace joedb
{
 /// \ingroup ui
 std::string base64_encode(const std::string &input);
 /// \ingroup ui
 std::string base64_decode(const std::string &input);
}

#endif
