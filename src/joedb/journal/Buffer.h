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
 class Buffer
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   static constexpr size_t buffer_size = (1 << 12);
   static constexpr size_t buffer_extra = 8;

   char buffer[buffer_size + buffer_extra];
   size_t index = 0;

  public:
   //////////////////////////////////////////////////////////////////////////
   void reset()
   //////////////////////////////////////////////////////////////////////////
   {
    index = 0;
   }

   //////////////////////////////////////////////////////////////////////////
   bool overflow() const
   //////////////////////////////////////////////////////////////////////////
   {
    return index >= buffer_size;
   }

   //////////////////////////////////////////////////////////////////////////
   template<typename T> void write(T x)
   //////////////////////////////////////////////////////////////////////////
   {
    JOEDB_ASSERT(index + sizeof(T) < buffer_size + buffer_extra);
    std::copy_n((const char *)&x, sizeof(T), buffer + index);
    index += sizeof(T);
   }

   //////////////////////////////////////////////////////////////////////////
   template<typename T> T read()
   //////////////////////////////////////////////////////////////////////////
   {
    JOEDB_ASSERT(index + sizeof(T) < buffer_size + buffer_extra);
    T result;
    std::copy_n(buffer + index, sizeof(T), (char *)&result);
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

     while ((x >> (8 * extra_bytes)) >= 32)
      extra_bytes++;

     write<uint8_t>((extra_bytes << 5) | (x >> (8 * extra_bytes)));

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
    while (extra_bytes--)
     result = T((result << 8) | read<uint8_t>());
    return result;
   }

   template<typename T> T read_strong_type()
   {
    return T(compact_read<typename std::underlying_type<T>::type>());
   }

   void write_reference(Record_Id id)
   {
    compact_write(to_underlying(id));
   }

   Record_Id read_reference()
   {
    return Record_Id(compact_read<std::underlying_type<Record_Id>::type>());
   }
 };
}

#endif
