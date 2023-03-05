#ifndef joedb_is_big_endian_declared
#define joedb_is_big_endian_declared

namespace joedb
{
 constexpr bool is_big_endian()
 {
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
  return true;
#elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
  return false;
#elif defined(_WIN32)
  return false;
#else
#error unknow endianness
#endif
 }
}

#endif
