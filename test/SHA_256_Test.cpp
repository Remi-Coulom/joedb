#include "joedb/journal/SHA_256.h"
#include "joedb/journal/Memory_File.h"

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
static const joedb::SHA_256::Hash abc_hash
/////////////////////////////////////////////////////////////////////////////
{
 {
  0xba7816bfU, 0x8f01cfeaU, 0x414140deU, 0x5dae2223U,
  0xb00361a3U, 0x96177a9cU, 0xb410ff61U, 0xf20015adU
 }
};

/////////////////////////////////////////////////////////////////////////////
TEST(SHA_256, sha_256)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::SHA_256 sha_256;
 sha_256.process_final_chunk("abc", 3);
 EXPECT_EQ(sha_256.get_hash(), abc_hash);
}

/////////////////////////////////////////////////////////////////////////////
TEST(SHA_256, file)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 file.write<char>('a');
 file.write<char>('b');
 file.write<char>('c');

 EXPECT_EQ(file.get_hash(), abc_hash);
}

/////////////////////////////////////////////////////////////////////////////
TEST(SHA_256, file_slice)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 file.write<char>('x');
 file.write<char>('x');
 file.write<char>('a');
 file.write<char>('b');
 file.write<char>('c');
 file.write<char>('d');
 file.write<char>('e');

 EXPECT_EQ(file.get_hash(2, 3), abc_hash);
}
