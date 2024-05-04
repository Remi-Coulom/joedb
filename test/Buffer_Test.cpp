#include "joedb/journal/Buffer.h"

#include "gtest/gtest.h"

#include <random>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 TEST(Buffer, basic)
 /////////////////////////////////////////////////////////////////////////////
 {
  Buffer buffer;

  buffer.write<uint8_t>(12);
  buffer.write<uint16_t>(123);
  buffer.write<uint32_t>(1234);
  buffer.write<uint64_t>(12345);

  buffer.reset();

  EXPECT_EQ(buffer.read<uint8_t>(), 12);
  EXPECT_EQ(buffer.read<uint16_t>(), 123);
  EXPECT_EQ(buffer.read<uint32_t>(), 1234);
  EXPECT_EQ(buffer.read<uint64_t>(), 12345);
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Buffer, compact)
 /////////////////////////////////////////////////////////////////////////////
 {
  Buffer buffer;

  buffer.compact_write<uint8_t>(12);
  buffer.compact_write<uint16_t>(123);
  buffer.compact_write<uint32_t>(1234);
  buffer.compact_write<uint64_t>(12345);

  buffer.reset();

  EXPECT_EQ(buffer.compact_read<uint8_t>(), 12);
  EXPECT_EQ(buffer.compact_read<uint16_t>(), 123);
  EXPECT_EQ(buffer.compact_read<uint32_t>(), 1234);
  EXPECT_EQ(buffer.compact_read<uint64_t>(), 12345);
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Buffer, many_compact)
 /////////////////////////////////////////////////////////////////////////////
 {
  Buffer buffer;

  for (uint64_t i = 0; i < 3000000; i += 1 + (i >> 16))
  {
   buffer.reset();
   buffer.compact_write<uint64_t>(i);
   buffer.reset();
   EXPECT_EQ(buffer.compact_read<uint64_t>(), i);
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 TEST(Buffer, many_small)
 /////////////////////////////////////////////////////////////////////////////
 {
  Buffer buffer;

  for (uint64_t i = 0; i < 3000000; i += 1 + (i >> 16))
  {
   buffer.reset();
   buffer.compact_write<uint64_t>(i & 0x1ff);
   buffer.reset();
   EXPECT_EQ(buffer.compact_read<uint64_t>(), i & 0x1ff);
  }
 }
}
