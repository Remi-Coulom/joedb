#include "joedb/journal/SHA_256.h"

#include "gtest/gtest.h"

/////////////////////////////////////////////////////////////////////////////
TEST(SHA_256, rotr)
/////////////////////////////////////////////////////////////////////////////
{
 EXPECT_EQ(joedb::rotr(0x12345678U,  4), 0x81234567U);
 EXPECT_EQ(joedb::rotr(0x00000001U,  1), 0x80000000U);
 EXPECT_EQ(joedb::rotr(0x80000000U,  1), 0x40000000U);
 EXPECT_EQ(joedb::rotr(0x80000000U,  4), 0x08000000U);
 EXPECT_EQ(joedb::rotr(0x80000000U, 28), 0x00000008U);
}

/////////////////////////////////////////////////////////////////////////////
TEST(SHA_256, sha_256)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::SHA_256 sha_256;

 uint32_t chunk[16];
 for (uint32_t i = 0; i < 16; i++)
  chunk[i] = i;

 sha_256.process_chunk(chunk);

 const std::array<uint32_t, 8> &h = sha_256.get_hash();
 EXPECT_EQ(h[0], 1746806733U);
}
