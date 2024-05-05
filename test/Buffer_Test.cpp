#include "joedb/journal/Buffer.h"

#include "gtest/gtest.h"

#include <random>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 TEST(Buffer, basic)
 /////////////////////////////////////////////////////////////////////////////
 {
  Buffer<12> buffer;
  buffer.index = 0;

  buffer.write<uint8_t>(12);
  buffer.write<uint16_t>(123);
  buffer.write<uint32_t>(1234);
  buffer.write<uint64_t>(12345);

  buffer.index = 0;

  EXPECT_EQ(buffer.read<uint8_t>(), 12U);
  EXPECT_EQ(buffer.read<uint16_t>(), 123U);
  EXPECT_EQ(buffer.read<uint32_t>(), 1234U);
  EXPECT_EQ(buffer.read<uint64_t>(), 12345U);
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Buffer, compact)
 /////////////////////////////////////////////////////////////////////////////
 {
  Buffer<12> buffer;
  buffer.index = 0;

  buffer.compact_write<uint8_t>(12);
  buffer.compact_write<uint16_t>(123);
  buffer.compact_write<uint32_t>(1234);
  buffer.compact_write<uint64_t>(12345);

  buffer.index = 0;

  EXPECT_EQ(buffer.compact_read<uint8_t>(), 12U);
  EXPECT_EQ(buffer.compact_read<uint16_t>(), 123U);
  EXPECT_EQ(buffer.compact_read<uint32_t>(), 1234U);
  EXPECT_EQ(buffer.compact_read<uint64_t>(), 12345U);
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Buffer, big_compact)
 /////////////////////////////////////////////////////////////////////////////
 {
  Buffer<12> buffer;

  const uint32_t magic = 190348190UL;

  buffer.index = 0;
  buffer.compact_write<uint32_t>(magic);
  buffer.index = 0;
  EXPECT_EQ(buffer.compact_read<uint32_t>(), magic);
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Buffer, many_compact)
 /////////////////////////////////////////////////////////////////////////////
 {
  Buffer<12> buffer;

  for (uint64_t i = 0; i < 3000000; i += 1 + (i >> 16))
  {
   buffer.index = 0;
   buffer.compact_write<uint64_t>(i);
   buffer.index = 0;
   EXPECT_EQ(buffer.compact_read<uint64_t>(), i);
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Buffer, many_small)
 /////////////////////////////////////////////////////////////////////////////
 {
  Buffer<12> buffer;

  for (uint64_t i = 0; i < 3000000; i += 1 + (i >> 16))
  {
   buffer.index = 0;
   buffer.compact_write<uint64_t>(i & 0x1ff);
   buffer.index = 0;
   EXPECT_EQ(buffer.compact_read<uint64_t>(), i & 0x1ff);
  }
 }
}
