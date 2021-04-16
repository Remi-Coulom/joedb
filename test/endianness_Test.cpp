#include "joedb/journal/File.h"
#include "joedb/journal/Readonly_Journal.h"
#include "joedb/interpreter/Database.h"
#include "joedb/concurrency/network_integers.h"

#include "gtest/gtest.h"

/////////////////////////////////////////////////////////////////////////////
TEST(endianness, reading)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::File file("endianness.joedb", joedb::Open_Mode::read_existing);
 joedb::Readonly_Journal journal(file);
 joedb::Database db;
 journal.replay_log(db);
 EXPECT_EQ(1, db.get_int8(1, 1, 1));
 EXPECT_EQ(258, db.get_int16(1, 1, 2));
 EXPECT_EQ(16909060, db.get_int32(1, 1, 3));
 EXPECT_EQ(72623859790382856, db.get_int64(1, 1, 4));
 EXPECT_EQ(123.0f, db.get_float32(1, 1, 5));
 EXPECT_EQ(456.0, db.get_float64(1, 1, 6));
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
   const int64_t n = i << shift;
   joedb::to_network(n, buffer);
   EXPECT_EQ(n, joedb::from_network(buffer));
  }
 }
}
