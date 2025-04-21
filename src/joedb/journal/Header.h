#ifndef joedb_Header_declared
#define joedb_Header_declared

#include <stdint.h>
#include <cstddef>

namespace joedb
{
 // @ingroup journal
 struct Header
 {
  int64_t checkpoint[4];
  uint32_t version;
  char joedb[5];
 };

 static_assert(offsetof(Header, checkpoint[0]) == 0);
 static_assert(offsetof(Header, checkpoint[1]) == 8);
 static_assert(offsetof(Header, checkpoint[2]) == 16);
 static_assert(offsetof(Header, checkpoint[3]) == 24);
 static_assert(offsetof(Header, version) == 32);
 static_assert(offsetof(Header, joedb) == 36);
}

#endif
