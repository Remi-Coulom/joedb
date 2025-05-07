#include "joedb/interpreted/Database.h"
#include "gtest/gtest.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 TEST(Interpreted_Test, database_errors)
 ////////////////////////////////////////////////////////////////////////////
 {
  joedb::Database db;
  EXPECT_ANY_THROW(db.insert_into(Table_Id(1), Record_Id(0)));
  db.create_table("person");
  EXPECT_EQ(db.find_field(Table_Id{2}, "toto"), Field_Id{0});
  EXPECT_EQ(db.get_field_name(Table_Id{2}, Field_Id{1}), "__unknown_field__");
  EXPECT_ANY_THROW(db.insert_into(Table_Id(1), Record_Id(-1)));
  EXPECT_NO_THROW(db.insert_into(Table_Id(1), Record_Id(0)));
  EXPECT_ANY_THROW(db.insert_into(Table_Id(2), Record_Id(0)));
  EXPECT_ANY_THROW(db.insert_vector(Table_Id(2), Record_Id(0), size_t(1)));
  EXPECT_NO_THROW(db.insert_vector(Table_Id(1), Record_Id(1), size_t(10)));
  EXPECT_ANY_THROW(db.insert_vector(Table_Id(1), Record_Id(-1), size_t(10)));
  EXPECT_ANY_THROW(db.delete_from(Table_Id(0), Record_Id(-1)));
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(Interpreted_Test, schema_errors)
 ////////////////////////////////////////////////////////////////////////////
 {
  joedb::Database_Schema schema;
  EXPECT_ANY_THROW(schema.get_fields(Table_Id(1)));
  EXPECT_ANY_THROW(schema.get_freedom(Table_Id(1)));
  EXPECT_ANY_THROW(schema.drop_table(Table_Id(1)));
  EXPECT_ANY_THROW(schema.rename_table(Table_Id(1), "toto"));
  EXPECT_ANY_THROW(schema.add_field(Table_Id(1), "toto", joedb::Type::int32()));
  EXPECT_ANY_THROW(schema.drop_field(Table_Id(1), Field_Id(1)));
  EXPECT_ANY_THROW(schema.rename_field(Table_Id(1), Field_Id(1), "toto"));
 }
}
