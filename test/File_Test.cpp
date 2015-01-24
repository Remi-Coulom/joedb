#include "File.h"
#include "gtest/gtest.h"

using namespace joedb;

static const uint64_t joedb_magic = 0x0000620165646A6FULL;

class File_Test: public::testing::Test
{
 protected:
  virtual void SetUp()
  {
   File file("existing.tmp", File::mode_t::create_new);
   file.write<uint64_t>(joedb_magic);
  }

  virtual void TearDown()
  {
   std::remove("existing.tmp");
   std::remove("new.tmp");
  }
};

/////////////////////////////////////////////////////////////////////////////
TEST_F(File_Test, open_failure)
{
 {
  File file("not_existing.tmp", File::mode_t::read_existing);
  EXPECT_FALSE(file.is_good());
 }
 {
  File file("not_existing.tmp", File::mode_t::write_existing);
  EXPECT_FALSE(file.is_good());
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST_F(File_Test, open_success)
{
 File file("existing.tmp", File::mode_t::read_existing);
 EXPECT_TRUE(file.is_good());
 EXPECT_EQ(file.get_mode(), File::mode_t::read_existing);

 File new_file("new.tmp", File::mode_t::create_new);
 EXPECT_TRUE(file.is_good());
 new_file.flush();
}

/////////////////////////////////////////////////////////////////////////////
TEST_F(File_Test, read_existing)
{
 File existing("existing.tmp", File::mode_t::read_existing);
 EXPECT_EQ(existing.read<uint64_t>(), joedb_magic);
}

/////////////////////////////////////////////////////////////////////////////
TEST_F(File_Test, read_write_integer)
{
 {
  File new_file("new.tmp", File::mode_t::create_new);
  new_file.write<uint8_t>(uint8_t(joedb_magic));
  new_file.set_position(0);
  EXPECT_EQ(new_file.read<uint8_t>(), uint8_t(joedb_magic));
 }
 {
  File new_file("new.tmp", File::mode_t::create_new);
  new_file.write<uint16_t>(uint16_t(joedb_magic));
  new_file.set_position(0);
  EXPECT_EQ(new_file.read<uint16_t>(), uint16_t(joedb_magic));
 }
 {
  File new_file("new.tmp", File::mode_t::create_new);
  new_file.write<uint32_t>(uint32_t(joedb_magic));
  new_file.set_position(0);
  EXPECT_EQ(new_file.read<uint32_t>(), uint32_t(joedb_magic));
 }
 {
  File new_file("new.tmp", File::mode_t::create_new);
  new_file.write<uint64_t>(uint64_t(joedb_magic));
  new_file.set_position(0);
  EXPECT_EQ(new_file.read<uint64_t>(), uint64_t(joedb_magic));
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST_F(File_Test, read_write_string)
{
 File new_file("new.tmp", File::mode_t::create_new);
 const std::string s("joedb!!!");
 new_file.write_string(s);
 new_file.set_position(0);
 EXPECT_EQ(new_file.read_string(), s);
}

/////////////////////////////////////////////////////////////////////////////
TEST_F(File_Test, position_test)
{
 File file("new.tmp", File::mode_t::create_new);
 EXPECT_EQ(0, file.get_position());

 file.set_position(uint64_t(-1));
 EXPECT_EQ(0, file.get_position());

 const int N = 100;
 for (int i = N; --i >= 0;)
  file.write<uint8_t>('x');
 EXPECT_EQ(N, file.get_position());

 const int pos = 12;
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
 File file("new.tmp", File::mode_t::create_new);
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
