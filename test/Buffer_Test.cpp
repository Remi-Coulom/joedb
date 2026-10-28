#include "joedb/journal/Buffer.h"

#include "gtest/gtest.h"

#include <limits>
#include <vector>
#include <algorithm>

namespace joedb
{
 template<typename T> class Compact_Unsigned_Test: public testing::Test {};
 using Compact_Types = testing::Types<uint8_t, uint16_t, uint32_t, uint64_t>;
 TYPED_TEST_SUITE(Compact_Unsigned_Test, Compact_Types);

 TYPED_TEST(Compact_Unsigned_Test, canonical_encoding_at_boundaries)
 {
  const uint64_t maximum = std::min<uint64_t>(std::numeric_limits<TypeParam>::max(),
   (uint64_t(1) << 61) - 1);
  std::vector<uint64_t> values{0};
  for (unsigned bit = 5; bit < 61; bit += 8)
   for (const uint64_t value: {(uint64_t(1) << bit) - 1, uint64_t(1) << bit})
    if (value <= maximum)
     values.push_back(value);
  values.push_back(maximum);
  for (const uint64_t value: values)
  {
   SCOPED_TRACE(value);
   uint64_t high = value;
   size_t extra = 0;
   while (high >= 32)
   {
    ++extra;
    high >>= 8;
   }
   std::vector<uint8_t> expected{uint8_t((extra << 5) | high)};
   for (size_t remaining = extra; remaining > 0; --remaining)
    expected.push_back(uint8_t(value >> (8 * (remaining - 1))));
   Buffer<12> buffer{};
   buffer.compact_write<TypeParam>(TypeParam(value));
   ASSERT_EQ(buffer.index, expected.size());
   for (size_t i = 0; i < expected.size(); ++i)
    EXPECT_EQ(uint8_t(buffer.data[i]), expected[i]);
   buffer.index = 0;
   EXPECT_EQ(buffer.compact_read<TypeParam>(), TypeParam(value));
   EXPECT_EQ(buffer.index, expected.size());
  }
 }

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

#ifdef NDEBUG
 static constexpr uint64_t many = 100000000;
#else
 static constexpr uint64_t many = 3000000;
#endif

 /////////////////////////////////////////////////////////////////////////////
 TEST(Buffer, many_compact)
 /////////////////////////////////////////////////////////////////////////////
 {
  Buffer<12> buffer;
  buffer.index = 0;
  buffer.write<uint64_t>(0);

  for (uint64_t i = 0; i < many; i += 1)
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
  buffer.index = 0;
  buffer.write<uint64_t>(0);

  for (uint64_t i = 0; i < many; i += 1)
  {
   buffer.index = 0;
   buffer.compact_write<uint64_t>(i & 0x1ff);
   buffer.index = 0;
   EXPECT_EQ(buffer.compact_read<uint64_t>(), i & 0x1ff);
  }
 }
}
