#include "File.h"
#include "gtest/gtest.h"

#include <random>

using namespace joedb;

static const uint64_t joedb_magic = 0x0000620165646A6FULL;

class File_Test: public::testing::Test
{
 protected:
  virtual void SetUp()
  {
   File file("existing.tmp", Open_Mode::create_new);
   file.write<uint64_t>(joedb_magic);
   file.write<bool>(false);
   file.write<bool>(true);
  }

  virtual void TearDown()
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

/////////////////////////////////////////////////////////////////////////////
TEST_F(File_Test, open_lock)
{
 std::remove("locked.tmp");
 {
  File locked_file_1("locked.tmp", Open_Mode::create_new);
  locked_file_1.write<int>(1234);

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
TEST_F(File_Test, read_write_integer)
{
 {
  std::remove("new.tmp");
  File new_file("new.tmp", Open_Mode::create_new);
  new_file.write<uint64_t>(joedb_magic);
  new_file.set_position(0);
  EXPECT_EQ(joedb_magic, new_file.read<uint64_t>());
 }

 std::random_device rd;
 std::mt19937_64 gen(rd());
 const int N = 1000;

 {
  std::remove("new.tmp");
  File new_file("new.tmp", Open_Mode::create_new);
  for (int i = N; --i >= 0;)
  {
   uint16_t value = uint16_t(gen());
   new_file.set_position(0);
   new_file.compact_write<uint16_t>(value);
   new_file.set_position(0);
   EXPECT_EQ(value, new_file.compact_read<uint16_t>());
  }
 }
 {
  std::remove("new.tmp");
  File new_file("new.tmp", Open_Mode::create_new);
  for (int i = N; --i >= 0;)
  {
   uint32_t value = uint32_t(gen());
   new_file.set_position(0);
   new_file.compact_write<uint32_t>(value);
   new_file.set_position(0);
   EXPECT_EQ(value, new_file.compact_read<uint32_t>());
  }
 }
 {
  std::remove("new.tmp");
  File new_file("new.tmp", Open_Mode::create_new);
  for (int i = N; --i >= 0;)
  {
   uint64_t value = uint64_t(gen()) & 0x1fffffffffffffffULL;
   new_file.set_position(0);
   new_file.compact_write<uint64_t>(value);
   new_file.set_position(0);
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
 EXPECT_EQ(0ULL, file.get_position());

 file.set_position(uint64_t(-1));
 EXPECT_EQ(0ULL, file.get_position());

 const uint64_t N = 100;
 for (int i = N; --i >= 0;)
  file.write<uint8_t>('x');
 EXPECT_EQ(N, file.get_position());

 const uint64_t pos = 12;
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
