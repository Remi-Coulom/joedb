#ifndef joedb_network_integers_declared
#define joedb_network_integers_declared

#include <stdint.h>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 inline int64_t from_network(const char *buffer)
 ////////////////////////////////////////////////////////////////////////////
 {
  return
   int64_t(uint8_t(buffer[7])) << 56 |
   int64_t(uint8_t(buffer[6])) << 48 |
   int64_t(uint8_t(buffer[5])) << 40 |
   int64_t(uint8_t(buffer[4])) << 32 |
   int64_t(uint8_t(buffer[3])) << 24 |
   int64_t(uint8_t(buffer[2])) << 16 |
   int64_t(uint8_t(buffer[1])) <<  8 |
   int64_t(uint8_t(buffer[0]));
 }

 ////////////////////////////////////////////////////////////////////////////
 inline void to_network(int64_t n, char *buffer)
 ////////////////////////////////////////////////////////////////////////////
 {
  buffer[0] = char(n      );
  buffer[1] = char(n >>  8);
  buffer[2] = char(n >> 16);
  buffer[3] = char(n >> 24);
  buffer[4] = char(n >> 32);
  buffer[5] = char(n >> 40);
  buffer[6] = char(n >> 48);
  buffer[7] = char(n >> 56);
 }

 ////////////////////////////////////////////////////////////////////////////
 inline uint32_t uint32_from_network(const char *buffer)
 ////////////////////////////////////////////////////////////////////////////
 {
  return
   uint32_t(uint8_t(buffer[3])) << 24 |
   uint32_t(uint8_t(buffer[2])) << 16 |
   uint32_t(uint8_t(buffer[1])) <<  8 |
   uint32_t(uint8_t(buffer[0]));
 }

 ////////////////////////////////////////////////////////////////////////////
 inline void uint32_to_network(uint32_t n, char *buffer)
 ////////////////////////////////////////////////////////////////////////////
 {
  buffer[0] = char(n      );
  buffer[1] = char(n >>  8);
  buffer[2] = char(n >> 16);
  buffer[3] = char(n >> 24);
 }
}

#endif
