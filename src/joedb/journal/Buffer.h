#ifndef joedb_Buffer_declared
#define joedb_Buffer_declared

#include "joedb/assert.h"
#include "joedb/index_types.h"

#include <stddef.h>
#include <stdint.h>
#include <algorithm>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 template<int log_size> class Buffer
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   static constexpr size_t size = (1 << log_size);
   static constexpr int64_t ssize = (1 << log_size);
   static constexpr size_t extra_size = 8;

   char data[size + extra_size];
   size_t index;

   //////////////////////////////////////////////////////////////////////////
   template<typename T> void write(T x)
   //////////////////////////////////////////////////////////////////////////
   {
    JOEDB_ASSERT(index + sizeof(T) < size + extra_size);
    std::copy_n((const char *)&x, sizeof(T), data + index);
    index += sizeof(T);
   }

   //////////////////////////////////////////////////////////////////////////
   template<typename T> T read()
   //////////////////////////////////////////////////////////////////////////
   {
    JOEDB_ASSERT(index + sizeof(T) < size + extra_size);
    T result;
    std::copy_n(data + index, sizeof(T), (char *)&result);
    index += sizeof(T);
    return result;
   }

   //////////////////////////////////////////////////////////////////////////
   template<typename T> void compact_write(T x)
   //////////////////////////////////////////////////////////////////////////
   {
    if (x < 0x20)
     write<uint8_t>(x);
    else if (x < 0x20 * 0x100)
    {
     write<uint8_t>(0x20 | (x >> 8));
     write<uint8_t>(x);
    }
    else
    {
     uint32_t extra_bytes = 2;

     while ((x >> (8 * extra_bytes)) >= 32 && extra_bytes < sizeof(T) - 1)
      extra_bytes++;

     write<uint8_t>((extra_bytes << 5) | (x >> (8 * extra_bytes)));
     write<uint8_t>(x >> (8 * --extra_bytes));
     write<uint8_t>(x >> (8 * --extra_bytes));
     while (extra_bytes)
      write<uint8_t>(x >> (8 * --extra_bytes));
    }
   }

   //////////////////////////////////////////////////////////////////////////
   template<typename T> T compact_read()
   //////////////////////////////////////////////////////////////////////////
   {
    const uint8_t first_byte = read<uint8_t>();
    int extra_bytes = first_byte >> 5;
    T result = first_byte & 0x1f;
    while (--extra_bytes >= 0)
     result = T((result << 8) | read<uint8_t>());
    return result;
   }
 };
}

#endif
