#include "joedb/journal/File_Hasher.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/journal/Writable_Journal.h"

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
TEST(SHA_256, sha_256_string)
/////////////////////////////////////////////////////////////////////////////
{
 EXPECT_EQ(joedb::File_Hasher::get_hash("abc"), abc_hash);
}

/////////////////////////////////////////////////////////////////////////////
TEST(SHA_256, file)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;

 {
  joedb::File_Buffer file_buffer(file);
  file_buffer.write<char>('a');
  file_buffer.write<char>('b');
  file_buffer.write<char>('c');
  file_buffer.flush();
 }

 EXPECT_EQ(joedb::File_Hasher::get_hash(file), abc_hash);
}

/////////////////////////////////////////////////////////////////////////////
TEST(SHA_256, file_slice)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 {
  joedb::File_Buffer file_buffer(file);
  file_buffer.write<char>('x');
  file_buffer.write<char>('x');
  file_buffer.write<char>('a');
  file_buffer.write<char>('b');
  file_buffer.write<char>('c');
  file_buffer.write<char>('d');
  file_buffer.write<char>('e');
  file_buffer.flush();
 }

 EXPECT_EQ(joedb::File_Hasher::get_hash(file, 2, 3), abc_hash);
 EXPECT_EQ(joedb::File_Hasher::get_fast_hash(file, 2, 3), abc_hash);
}

/////////////////////////////////////////////////////////////////////////////
TEST(SHA_256, fast_hash_coverage)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 {
  joedb::File_Buffer file_buffer(file);
  const size_t size = 1 << 20;
  for (size_t i = 0; i < size; i++)
   file_buffer.write<uint32_t>(uint32_t(i));
  file_buffer.flush();
 }
 joedb::File_Hasher::get_fast_hash(file, 0, file.get_size());
}

/////////////////////////////////////////////////////////////////////////////
static const joedb::SHA_256::Hash big_hash
/////////////////////////////////////////////////////////////////////////////
{
 {
  1375295911, 2979956878, 2035804433, 1588190633,
  2991985271, 583611159, 3820768175, 909004044
 }
};

/////////////////////////////////////////////////////////////////////////////
TEST(SHA_256, journal)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;

 {
  joedb::Writable_Journal journal(file);
  const int64_t size = 12345678;
  for (int64_t i = 0; i < size; i++)
   journal.timestamp(i);
  journal.soft_checkpoint();
 }

 joedb::Readonly_Journal journal(file);
 EXPECT_EQ(41, journal.get_position());

 const joedb::SHA_256::Hash hash = joedb::Journal_Hasher::get_fast_hash
 (
  journal,
  journal.get_checkpoint() - 123
 );

 EXPECT_EQ(hash, big_hash);
 EXPECT_EQ(41, journal.get_position());
}
