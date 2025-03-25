#include "joedb/interpreter/Database.h"
#include "gtest/gtest.h"

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
TEST(Interpreted_Test, database_errors)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::interpreter::Database db;
 EXPECT_ANY_THROW(db.insert_into(Table_Id(1), Record_Id(1)));
 db.create_table("person");
 EXPECT_ANY_THROW(db.insert_into(Table_Id(1), Record_Id(0)));
 EXPECT_NO_THROW(db.insert_into(Table_Id(1), Record_Id(1)));
 EXPECT_ANY_THROW(db.insert_into(Table_Id(2), Record_Id(1)));
 EXPECT_ANY_THROW(db.insert_vector(Table_Id(2), Record_Id(1), size_t(1)));
 EXPECT_NO_THROW(db.insert_vector(Table_Id(1), Record_Id(2), size_t(10)));
 EXPECT_ANY_THROW(db.insert_vector(Table_Id(1), Record_Id(0), size_t(10)));
 EXPECT_ANY_THROW(db.delete_from(Table_Id(0), Record_Id(0)));
}

/////////////////////////////////////////////////////////////////////////////
TEST(Interpreted_Test, schema_errors)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::interpreter::Database_Schema schema;
 EXPECT_ANY_THROW(schema.get_fields(Table_Id(1)));
 EXPECT_ANY_THROW(schema.get_freedom(Table_Id(1)));
 EXPECT_ANY_THROW(schema.drop_table(Table_Id(1)));
 EXPECT_ANY_THROW(schema.rename_table(Table_Id(1), "toto"));
 EXPECT_ANY_THROW(schema.add_field(Table_Id(1), "toto", joedb::Type::int32()));
 EXPECT_ANY_THROW(schema.drop_field(Table_Id(1), Field_Id(1)));
 EXPECT_ANY_THROW(schema.rename_field(Table_Id(1), Field_Id(1), "toto"));
}
