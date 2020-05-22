#ifndef joedb_base64_declared
#define joedb_base64_declared

#include <string>

namespace joedb
{
 std::string base64_encode(const std::string &input);
 std::string base64_decode(const std::string &input);
}

#endif
