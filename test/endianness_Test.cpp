#include "joedb/journal/File.h"
#include "joedb/journal/Readonly_Journal.h"
#include "joedb/interpreter/Database.h"

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
 joedb::interpreter::Database db;
 journal.replay_log(db);
 EXPECT_EQ(1, db.get_int8(Table_Id(1), Record_Id(1), Field_Id(1)));
 EXPECT_EQ(258, db.get_int16(Table_Id(1), Record_Id(1), Field_Id(2)));
 EXPECT_EQ(16909060, db.get_int32(Table_Id(1), Record_Id(1), Field_Id(3)));
 EXPECT_EQ(72623859790382856, db.get_int64(Table_Id(1), Record_Id(1), Field_Id(4)));
 EXPECT_EQ(123.0f, db.get_float32(Table_Id(1), Record_Id(1), Field_Id(5)));
 EXPECT_EQ(456.0, db.get_float64(Table_Id(1), Record_Id(1), Field_Id(6)));
}
