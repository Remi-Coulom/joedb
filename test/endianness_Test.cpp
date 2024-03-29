#include "joedb/journal/File.h"
#include "joedb/journal/Readonly_Journal.h"
#include "joedb/interpreter/Database.h"
#include "joedb/concurrency/network_integers.h"

#include "gtest/gtest.h"

/////////////////////////////////////////////////////////////////////////////
TEST(endianness, reading)
/////////////////////////////////////////////////////////////////////////////
{
 using joedb::Table_Id;
 using joedb::Record_Id;
 using joedb::Field_Id;

 joedb::File file("endianness.joedb", joedb::Open_Mode::read_existing);
 joedb::Readonly_Journal journal(file);
 joedb::Database db;
 journal.replay_log(db);
 EXPECT_EQ(1, db.get_int8(Table_Id(1), Record_Id(1), Field_Id(1)));
 EXPECT_EQ(258, db.get_int16(Table_Id(1), Record_Id(1), Field_Id(2)));
 EXPECT_EQ(16909060, db.get_int32(Table_Id(1), Record_Id(1), Field_Id(3)));
 EXPECT_EQ(72623859790382856, db.get_int64(Table_Id(1), Record_Id(1), Field_Id(4)));
 EXPECT_EQ(123.0f, db.get_float32(Table_Id(1), Record_Id(1), Field_Id(5)));
 EXPECT_EQ(456.0, db.get_float64(Table_Id(1), Record_Id(1), Field_Id(6)));
}

/////////////////////////////////////////////////////////////////////////////
TEST(endianness, network_integers)
/////////////////////////////////////////////////////////////////////////////
{
 for (int64_t i = -500; i <= +500; i++)
 {
  char buffer[8];
  for (int shift = 0; shift < 56; shift++)
  {
   const int64_t n = int64_t(uint64_t(i) << shift);
   joedb::to_network(n, buffer);
   EXPECT_EQ(n, joedb::from_network(buffer));
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(endianness, uint32_t)
/////////////////////////////////////////////////////////////////////////////
{
 for (uint32_t i = 0; i <= +500; i++)
 {
  char buffer[4];
  for (int shift = 0; shift < 24; shift++)
  {
   const uint32_t n = i << shift;
   joedb::uint32_to_network(n, buffer);
   EXPECT_EQ(n, joedb::uint32_from_network(buffer));
  }
 }
}
