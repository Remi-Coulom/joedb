#ifndef joedb_Transparent_String_Hash_declared
#define joedb_Transparent_String_Hash_declared

#include <functional>
#include <string_view>

namespace joedb
{
 struct Transparent_String_Hash: std::hash<std::string_view>
 {
  using is_transparent = void;
 };
}

#endif
