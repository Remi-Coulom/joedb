#ifndef joedb_is_big_endian_declared
#define joedb_is_big_endian_declared

#include <stdint.h>

namespace joedb
{
 inline uint8_t is_big_endian()
 {
  const uint16_t n = 0x0100;
  return *(const uint8_t *)&n;
 }
}

#endif
