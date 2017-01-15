#include "gtest/gtest.h"

#include "File.h"
#include "Readonly_Journal.h"
#include "Database.h"

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
}
