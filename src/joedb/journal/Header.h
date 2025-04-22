#ifndef joedb_Header_declared
#define joedb_Header_declared

#include <stdint.h>
#include <cstddef>
#include <array>

namespace joedb
{
 // @ingroup journal
 struct Header
 {
  std::array<int64_t, 4> checkpoint;
  uint32_t version;
  std::array<char, 5> signature;

  static constexpr std::array<char, 5> joedb{'j', 'o', 'e', 'd', 'b'};
  static constexpr int64_t ssize = 41;
  static constexpr size_t size = ssize;
 };

 static_assert(offsetof(Header, checkpoint) == 0);
 static_assert(offsetof(Header, version) == 32);
 static_assert(offsetof(Header, signature) == 36);
 static_assert(sizeof(std::array<char, 5>) == 5);
}

#endif
