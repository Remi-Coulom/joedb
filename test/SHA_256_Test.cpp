#include "joedb/journal/get_hash.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/journal/Writable_Journal.h"

#include "Test_File.h"

#include "gtest/gtest.h"

#include <vector>

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
 EXPECT_EQ(joedb::get_full_hash("abc"), abc_hash);
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

 EXPECT_EQ(joedb::get_full_hash(file), abc_hash);
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

 EXPECT_EQ(joedb::get_full_hash(file, 2, 5), abc_hash);
 EXPECT_EQ(joedb::get_fast_hash(file, 2, 5), abc_hash);
}

/////////////////////////////////////////////////////////////////////////////
TEST(SHA_256, file_with_short_reads)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Test_File file;
 file.get_data() = std::string(2048, 'x');
 file.get_data().back() = 'y';

 EXPECT_EQ
 (
  joedb::get_full_hash(file),
  joedb::get_full_hash(file.get_data())
 );
}

/////////////////////////////////////////////////////////////////////////////
TEST(SHA_256, fast_hash_with_short_reads)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Test_File short_read_file;
 short_read_file.get_data() = std::string(4 << 20, 'x');
 short_read_file.get_data().back() = 'y';

 joedb::Memory_File file;
 file.get_data() = short_read_file.get_data();

 EXPECT_EQ
 (
  joedb::get_fast_hash(short_read_file, 0, short_read_file.get_size()),
  joedb::get_fast_hash(file, 0, file.get_size())
 );
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
 joedb::get_fast_hash(file, 0, file.get_size());
}

/////////////////////////////////////////////////////////////////////////////
static const joedb::SHA_256::Hash big_hash
/////////////////////////////////////////////////////////////////////////////
{
 {
  113130918, 2435694586, 4004299373, 4110564913,
  4160222598, 3194168673, 795849175, 2699492686
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

 const joedb::SHA_256::Hash hash = joedb::get_fast_hash
 (
  journal.get_file(),
  joedb::Header::ssize,
  journal.get_checkpoint() - 123
 );

 EXPECT_EQ(hash, big_hash);
 EXPECT_EQ(41, journal.get_position());
}

/////////////////////////////////////////////////////////////////////////////
TEST(SHA_256, empty)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;

 {
  joedb::Writable_Journal journal(file);
  journal.timestamp(0);
  journal.soft_checkpoint();
 }

 EXPECT_EQ(
  joedb::get_full_hash(file, joedb::Header::ssize, joedb::Header::ssize),
  joedb::get_full_hash("")
 );
}

TEST(SHA_256, padding_and_block_boundaries)
{
 struct Case {size_t size; joedb::SHA_256::Hash hash;};
 const Case cases[] =
 {
  {0, {0xe3b0c442U, 0x98fc1c14U, 0x9afbf4c8U, 0x996fb924U, 0x27ae41e4U, 0x649b934cU, 0xa495991bU, 0x7852b855U}},
  {55, {0x9f4390f8U, 0xd30c2dd9U, 0x2ec9f095U, 0xb65e2b9aU, 0xe9b0a925U, 0xa5258e24U, 0x1c9f1e91U, 0x0f734318U}},
  {56, {0xb35439a4U, 0xac6f0948U, 0xb6d6f9e3U, 0xc6af0f5fU, 0x590ce20fU, 0x1bde7090U, 0xef797068U, 0x6ec6738aU}},
  {63, {0x7d3e74a0U, 0x5d7db15bU, 0xce4ad9ecU, 0x0658ea98U, 0xe3f06eeeU, 0xcf16b4c6U, 0xfff2da45U, 0x7ddc2f34U}},
  {64, {0xffe054feU, 0x7ae0cb6dU, 0xc65c3af9U, 0xb61d5209U, 0xf439851dU, 0xb43d0ba5U, 0x997337dfU, 0x154668ebU}},
  {65, {0x635361c4U, 0x8bb9eab1U, 0x4198e76eU, 0xa8ab7f1aU, 0x41685d6aU, 0xd62aa914U, 0x6d301d4fU, 0x17eb0ae0U}},
  {127, {0xc57e9278U, 0xaf78fa3cU, 0xab38667bU, 0xef4ce29dU, 0x783787a2U, 0xf731d4e1U, 0x2200270fU, 0x0c32320aU}},
  {128, {0x6836cf13U, 0xbac400e9U, 0x105071cdU, 0x6af47084U, 0xdfacad4eU, 0x5e302c94U, 0xbfed24e0U, 0x13afb73eU}},
  {129, {0xc12cb024U, 0xa2e5551cU, 0xca0e08fcU, 0xe8f1c5e3U, 0x14555cc3U, 0xfef6329eU, 0xe994a3dbU, 0x752166aeU}},
  {131071, {0x115b5799U, 0x5e13fe27U, 0xba1077d6U, 0x31a7fa51U, 0x82fbb798U, 0xac0193e8U, 0x79a75ebbU, 0x3680732cU}},
  {131073, {0x7e009ea4U, 0xef882e38U, 0x5b3c0bcbU, 0xbfa8d009U, 0xbb0a633bU, 0xdd764415U, 0xc09182eeU, 0x0e75da73U}},
 };
 for (const auto &test: cases)
 {
  SCOPED_TRACE(test.size);
  const std::string data(test.size, 'a');
  EXPECT_EQ(joedb::get_full_hash(data), test.hash);
 }
}

TEST(SHA_256, exact_hashing_buffer_multiple)
{
 struct Case {size_t size; joedb::SHA_256::Hash hash;};
 const Case cases[] =
 {
  {131072, {0xb44ffb72U, 0xfcc25967U, 0x6bd80495U, 0xfef1b44bU, 0x808ca8f1U, 0xffe1b170U, 0x6a4d7911U, 0xb0e31f11U}},
  {262144, {0xdd3dde87U, 0x623d9a6bU, 0x354c68c9U, 0x43d189c8U, 0x9c63652dU, 0x945e7bbdU, 0xf0986caeU, 0x91a49521U}},
  {393216, {0xcef254beU, 0x676d5530U, 0x8d292624U, 0x8a1237b3U, 0x906e478cU, 0xef15e76aU, 0x8e304849U, 0x97e86459U}},
 };
 for (const auto &test: cases)
 {
  SCOPED_TRACE(test.size);
  EXPECT_EQ(joedb::get_full_hash(std::string(test.size, 'a')), test.hash);
 }
}
