#include "joedb/journal/File.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/journal/Portable_File.h"

#include "gtest/gtest.h"

#include <random>
#include <cstdio>
#include <cstring>

using namespace joedb;

static const uint64_t joedb_magic = 0x0000620165646A6FULL;

class File_Test: public::testing::Test
{
 protected:
  void SetUp() override
  {
   File file("existing.tmp", Open_Mode::create_new);
   file.write<uint64_t>(joedb_magic);
   file.write<bool>(false);
   file.write<bool>(true);
   file.flush();
  }

  void TearDown() override
  {
   std::remove("locked.tmp");
   std::remove("existing.tmp");
   std::remove("new.tmp");
  }
};

/////////////////////////////////////////////////////////////////////////////
TEST_F(File_Test, open_failure)
{
 EXPECT_ANY_THROW
 (
  File file("not_existing.tmp", Open_Mode::read_existing)
 );

 EXPECT_ANY_THROW
 (
  File file("not_existing.tmp", Open_Mode::write_existing)
 );

 EXPECT_ANY_THROW
 (
  File file("existing.tmp", Open_Mode::create_new)
 );
}

#ifndef JOEDB_FILE_IS_PORTABLE_FILE
/////////////////////////////////////////////////////////////////////////////
TEST_F(File_Test, open_lock)
{
 std::remove("locked.tmp");
 {
  File locked_file_1("locked.tmp", Open_Mode::create_new);
  locked_file_1.write<int>(1234);
  locked_file_1.flush();

  EXPECT_ANY_THROW
  (
   File locked_file_2("locked.tmp", Open_Mode::write_existing)
  );
 }
 {
  File locked_file_1("locked.tmp", Open_Mode::write_existing);

  EXPECT_ANY_THROW
  (
   File locked_file_2("locked.tmp", Open_Mode::write_existing)
  );
 }
}
#endif

#if 0
#ifdef JOEDB_FILE_IS_POSIX_FILE
/////////////////////////////////////////////////////////////////////////////
TEST(File, no_space_left)
{
 const char * const file_name = "test.small_disk/test.joedb";
 const size_t max_size = 196605;

 using Test_File = joedb::Posix_File;
// using Test_File = joedb::Portable_File;

 {
  std::remove(file_name);
  Test_File file(file_name, Open_Mode::create_new);
  std::string s(max_size, 'x');
  file.write_string(s);
  file.flush();
 }

 {
  std::remove(file_name);
  Test_File file(file_name, Open_Mode::create_new);
  std::string s(max_size, 'x');
  file.write_string(s);
  file.flush();
  file.write<int32_t>(1234);
  EXPECT_ANY_THROW(file.flush());
 }
}
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
TEST_F(File_Test, open_success)
{
 {
  File file("existing.tmp", Open_Mode::read_existing);
  EXPECT_EQ(file.get_mode(), Open_Mode::read_existing);
 }

 {
  std::remove("new.tmp");
  File new_file("new.tmp", Open_Mode::create_new);
  new_file.flush();
  EXPECT_EQ(new_file.get_mode(), Open_Mode::create_new);
 }

 {
  std::remove("new.tmp");
  File new_file("new.tmp", Open_Mode::write_existing_or_create_new);
  new_file.flush();
  EXPECT_EQ(new_file.get_mode(), Open_Mode::create_new);
 }

 {
  File new_file("new.tmp", Open_Mode::write_existing_or_create_new);
  new_file.flush();
  EXPECT_EQ(new_file.get_mode(), Open_Mode::write_existing);
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST_F(File_Test, read_existing)
{
 File existing("existing.tmp", Open_Mode::read_existing);
 EXPECT_EQ(existing.read<uint64_t>(), joedb_magic);
 EXPECT_EQ(existing.read<bool>(), false);
 EXPECT_EQ(existing.read<bool>(), true);
}

/////////////////////////////////////////////////////////////////////////////
TEST_F(File_Test, long_consecutive_read_and_writes)
/////////////////////////////////////////////////////////////////////////////
{
 const int64_t N = 1000000;

 std::remove("new.tmp");
 File file("new.tmp", Open_Mode::create_new);

 for (int64_t i = 0; i < N; i++)
  file.write<int64_t>(i);

 file.set_position(0);
 for (int64_t i = 0; i < N; i++)
 {
  const int64_t x = file.read<int64_t>();
  EXPECT_EQ(i, x);
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST_F(File_Test, read_write_integer)
{
 {
  std::remove("new.tmp");
  File new_file("new.tmp", Open_Mode::create_new);
  new_file.write<uint64_t>(joedb_magic);
  new_file.set_position(0);
  EXPECT_EQ(joedb_magic, new_file.read<uint64_t>());
 }

#if 0
 std::random_device rd; // not supported by valgrind
 std::mt19937_64 gen(rd());
#else
 std::mt19937_64 gen(0);
#endif
 const int N = 1000;

 {
  std::remove("new.tmp");
  File new_file("new.tmp", Open_Mode::create_new);
  for (int i = 0; i < N; i++)
  {
   uint16_t value = uint16_t(gen());
   new_file.set_position(i);
   new_file.compact_write<uint16_t>(value);
   new_file.set_position(i);
   EXPECT_EQ(value, new_file.compact_read<uint16_t>());
  }
 }
 {
  std::remove("new.tmp");
  File new_file("new.tmp", Open_Mode::create_new);
  for (int i = 0; i < N; i++)
  {
   uint32_t value = uint32_t(gen());
   new_file.set_position(i);
   new_file.compact_write<uint32_t>(value);
   new_file.set_position(i);
   EXPECT_EQ(value, new_file.compact_read<uint32_t>());
  }
 }
 {
  std::remove("new.tmp");
  File new_file("new.tmp", Open_Mode::create_new);
  for (int i = 0; i < N; i++)
  {
   uint64_t value = uint64_t(gen()) & 0x1fffffffffffffffULL;
   new_file.set_position(i);
   new_file.compact_write<uint64_t>(value);
   new_file.set_position(i);
   EXPECT_EQ(value, new_file.compact_read<uint64_t>());
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST_F(File_Test, read_write_string)
{
 std::remove("new.tmp");
 File new_file("new.tmp", Open_Mode::create_new);
 const std::string s("joedb!!!");
 new_file.write_string(s);
 new_file.set_position(0);
 EXPECT_EQ(new_file.read_string(), s);
}

/////////////////////////////////////////////////////////////////////////////
TEST_F(File_Test, position_test)
{
 std::remove("new.tmp");
 File file("new.tmp", Open_Mode::create_new);
 EXPECT_EQ(0LL, file.get_position());

// That test was not correct
// https://en.cppreference.com/w/cpp/io/c/fseek
// "POSIX allows seeking beyond the existing end of file..."
// file.set_position(size_t(-1));
// EXPECT_EQ(0LL, file.get_position());

 const int64_t N = 100;
 for (int i = N; --i >= 0;)
  file.write<uint8_t>('x');
 EXPECT_EQ(N, file.get_position());

 const int64_t pos = 12;
 file.set_position(pos);
 EXPECT_EQ(pos, file.get_position());

 const uint8_t x = file.read<uint8_t>();
 EXPECT_EQ('x', x);
 EXPECT_EQ(pos + 1, file.get_position());

 file.set_position(N + 2);
 EXPECT_EQ(N + 2, file.get_position());
 file.write<uint8_t>('x');
 file.set_position(N + 1);
 const uint8_t c = file.read<uint8_t>();
 EXPECT_EQ(0, c);
 EXPECT_FALSE(file.is_end_of_file());
 EXPECT_EQ(N + 2, file.get_position());
}

/////////////////////////////////////////////////////////////////////////////
TEST_F(File_Test, eof)
{
 std::remove("new.tmp");
 File file("new.tmp", Open_Mode::create_new);
 EXPECT_FALSE(file.is_end_of_file());

 file.read<uint8_t>();
 EXPECT_TRUE(file.is_end_of_file());

 file.set_position(0);
 const int N = 100000;
 for (int i = N; --i >= 0;)
  file.write<uint8_t>('x');
 EXPECT_FALSE(file.is_end_of_file());

 file.set_position(N - 1);
 EXPECT_FALSE(file.is_end_of_file());

 uint8_t c = file.read<uint8_t>();
 EXPECT_EQ('x', c);
 EXPECT_FALSE(file.is_end_of_file());

 file.read<uint8_t>();
 EXPECT_TRUE(file.is_end_of_file());
}

/////////////////////////////////////////////////////////////////////////////
TEST_F(File_Test, flush)
/////////////////////////////////////////////////////////////////////////////
{
#ifndef JOEDB_PORTABLE
 std::remove("new.tmp");
 File file1("new.tmp", Open_Mode::shared_write);
 EXPECT_TRUE(file1.is_shared());
 file1.write<int32_t>(1234);
 file1.flush();

 File file2("new.tmp", Open_Mode::shared_write);
 file2.set_position(0);
 EXPECT_EQ(1234, file2.read<int32_t>());
#endif
}

/////////////////////////////////////////////////////////////////////////////
static void perf(size_t size)
/////////////////////////////////////////////////////////////////////////////
{
 File file("new.tmp", Open_Mode::create_new);
 const size_t total_size = 10000000;
 const int N = int(total_size / (size + 1));
 std::string s(size, ' ');

 for (int i = N; --i >= 0;)
  file.write_string(s);
}

TEST_F(File_Test, write_data_perf100000) {perf(100000);}
TEST_F(File_Test, write_data_perf10000) {perf(10000);}
TEST_F(File_Test, write_data_perf1000) {perf(1000);}
TEST_F(File_Test, write_data_perf100) {perf(100);}
TEST_F(File_Test, write_data_perf10) {perf(10);}
TEST_F(File_Test, write_data_perf5) {perf(5);}
TEST_F(File_Test, write_data_perf4) {perf(4);}
TEST_F(File_Test, write_data_perf3) {perf(3);}
TEST_F(File_Test, write_data_perf2) {perf(2);}
TEST_F(File_Test, write_data_perf1) {perf(1);}
TEST_F(File_Test, write_data_perf0) {perf(0);}

/////////////////////////////////////////////////////////////////////////////
TEST(File, write_data)
/////////////////////////////////////////////////////////////////////////////
{
 std::string input;
 input.resize(10000);
 std::mt19937_64 gen(0);

 for (size_t i = 0; i < input.size(); i++)
  input[i] = char(gen());

 for (size_t n = 1; n <= input.size(); n++)
 {
  Memory_File file;

  size_t offset = size_t(gen()) & 0x7ffULL;
  file.write_data(&input[1], offset);
  file.write_data(&input[0], n);
  file.set_position(int64_t(offset));
  std::string output(n, 0);
  file.read_data(&output[0], n);
  ASSERT_EQ(0, std::memcmp(&input[0], &output[0], n)) << "n = " << n;
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(File, portable)
/////////////////////////////////////////////////////////////////////////////
{
 std::remove("test.joedb");

 EXPECT_ANY_THROW
 (
  joedb::Portable_File("test.joedb", joedb::Open_Mode::read_existing)
 );

 {
  joedb::Portable_File
  (
   "test.joedb",
   joedb::Open_Mode::write_existing_or_create_new
  );
 }

 EXPECT_ANY_THROW
 (
  joedb::Portable_File("test.joedb", joedb::Open_Mode::create_new)
 );

 {
  joedb::Portable_File("test.joedb", joedb::Open_Mode::read_existing);
 }

 {
  joedb::Portable_File("test.joedb", joedb::Open_Mode::write_existing);
 }

 EXPECT_ANY_THROW
 (
  joedb::Portable_File("test.joedb", joedb::Open_Mode::shared_write)
 );

 EXPECT_ANY_THROW
 (
  joedb::Portable_File("test.joedb", joedb::Open_Mode::write_lock)
 );

 std::remove("test.joedb");
}
