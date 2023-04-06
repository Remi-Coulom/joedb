#include "joedb/interpreter/Database.h"
#include "gtest/gtest.h"

#include <fstream>
#include <sstream>

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
TEST(Interpreted_Test, database_errors)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Database db;
 EXPECT_ANY_THROW(db.insert_into(1, 1));
 db.create_table("person");
 EXPECT_ANY_THROW(db.insert_into(1, 0));
 EXPECT_NO_THROW(db.insert_into(1, 1));
 EXPECT_ANY_THROW(db.insert_into(2, 1));
 EXPECT_ANY_THROW(db.insert_vector(2, 1, 1));
 EXPECT_NO_THROW(db.insert_vector(1, 2, 10));
 EXPECT_ANY_THROW(db.insert_vector(1, 0, 10));
 EXPECT_ANY_THROW(db.delete_from(0, 0));
}

/////////////////////////////////////////////////////////////////////////////
TEST(Interpreted_Test, schema_errors)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Database_Schema schema;
 EXPECT_ANY_THROW(schema.get_fields(1));
 EXPECT_ANY_THROW(schema.get_freedom(1));
 EXPECT_ANY_THROW(schema.drop_table(1));
 EXPECT_ANY_THROW(schema.rename_table(1, "toto"));
 EXPECT_ANY_THROW(schema.add_field(1, "toto", joedb::Type::int32()));
 EXPECT_ANY_THROW(schema.drop_field(1, 1));
 EXPECT_ANY_THROW(schema.rename_field(1, 1, "toto"));
}
