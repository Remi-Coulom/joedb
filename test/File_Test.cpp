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
   file.write_uint64(joedb_magic);
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
TEST_F(File_Test, read_write)
{
 File existing("existing.tmp", File::mode_t::read_existing);
 EXPECT_EQ(existing.read_uint64(), joedb_magic);

 {
  File new_file("new.tmp", File::mode_t::create_new);
  new_file.write_uint8(uint8_t(joedb_magic));
  new_file.seek(0);
  EXPECT_EQ(new_file.read_uint8(), uint8_t(joedb_magic));
 }
 {
  File new_file("new.tmp", File::mode_t::create_new);
  new_file.write_uint16(uint16_t(joedb_magic));
  new_file.seek(0);
  EXPECT_EQ(new_file.read_uint16(), uint16_t(joedb_magic));
 }
 {
  File new_file("new.tmp", File::mode_t::create_new);
  new_file.write_uint32(uint32_t(joedb_magic));
  new_file.seek(0);
  EXPECT_EQ(new_file.read_uint32(), uint32_t(joedb_magic));
 }
}
