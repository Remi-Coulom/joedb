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
 sha_256.process_final_chunk("abc", 3);
 const std::array<uint32_t, 8> &h = sha_256.get_hash();
 EXPECT_EQ(h[0], 0xba7816bfU);
 EXPECT_EQ(h[1], 0x8f01cfeaU);
 EXPECT_EQ(h[2], 0x414140deU);
 EXPECT_EQ(h[3], 0x5dae2223U);
 EXPECT_EQ(h[4], 0xb00361a3U);
 EXPECT_EQ(h[5], 0x96177a9cU);
 EXPECT_EQ(h[6], 0xb410ff61U);
 EXPECT_EQ(h[7], 0xf20015adU);
}
