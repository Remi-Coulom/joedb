#include "joedb/interpreted/Database.h"
#include "gtest/gtest.h"
#include <limits>

namespace joedb
{
 TEST(Interpreted_Test, rejects_unknown_field_without_changing_data)
 {
  Database db;
  db.create_table("items");
  db.add_field(Table_Id(1), "value", Type::int32());
  db.insert_into(Table_Id(1), Record_Id(0));
  db.update_int32(Table_Id(1), Record_Id(0), Field_Id(1), 42);
  EXPECT_THROW(db.update_int32(Table_Id(1), Record_Id(0), Field_Id(2), 100), Exception);
  EXPECT_THROW(db.get_int32(Table_Id(1), Record_Id(0), Field_Id(2)), Exception);
  EXPECT_EQ(db.get_int32(Table_Id(1), Record_Id(0), Field_Id(1)), 42);
 }

 TEST(Interpreted_Test, exhausted_table_ids_are_reported)
 {
  Database db;
  const auto maximum = std::numeric_limits<underlying_type<Table_Id>::type>::max();
  for (uint32_t id = 1; id <= maximum; ++id)
  {
   db.create_table("temporary");
   db.drop_table(Table_Id(id));
  }
  EXPECT_THROW(db.create_table("overflow"), Exception);
  EXPECT_TRUE(db.get_tables().empty());
 }

 TEST(Interpreted_Test, exhausted_field_ids_are_reported)
 {
  Database db;
  db.create_table("items");
  const auto maximum = std::numeric_limits<underlying_type<Field_Id>::type>::max();
  for (uint32_t id = 1; id <= maximum; ++id)
  {
   db.add_field(Table_Id(1), "temporary", Type::int32());
   db.drop_field(Table_Id(1), Field_Id(id));
  }
  EXPECT_THROW(db.add_field(Table_Id(1), "overflow", Type::int32()), Exception);
  EXPECT_TRUE(db.get_fields(Table_Id(1)).empty());
 }

 TEST(Interpreted_Test, vector_update_rejects_deleted_middle_record)
 {
  Database db;
  db.create_table("items");
  db.add_field(Table_Id(1), "value", Type::int32());
  db.insert_vector(Table_Id(1), Record_Id(0), 3);
  db.update_int32(Table_Id(1), Record_Id(0), Field_Id(1), 7);
  db.update_int32(Table_Id(1), Record_Id(2), Field_Id(1), 9);
  db.delete_from(Table_Id(1), Record_Id(1));
  const int32_t values[] = {100, 200, 300};
  EXPECT_THROW(db.update_vector_int32(Table_Id(1), Record_Id(0), Field_Id(1), 3, values), Exception);
  EXPECT_EQ(db.get_int32(Table_Id(1), Record_Id(0), Field_Id(1)), 7);
  EXPECT_EQ(db.get_int32(Table_Id(1), Record_Id(2), Field_Id(1)), 9);
 }

 TEST(Interpreted_Test, empty_vector_update_is_noop)
 {
  Database db;
  db.create_table("items");
  db.add_field(Table_Id(1), "value", Type::int32());
  db.insert_into(Table_Id(1), Record_Id(0));
  db.update_int32(Table_Id(1), Record_Id(0), Field_Id(1), 42);
  EXPECT_NO_THROW(db.update_vector_int32(Table_Id(1), Record_Id(0), Field_Id(1), 0, nullptr));
  EXPECT_EQ(db.get_int32(Table_Id(1), Record_Id(0), Field_Id(1)), 42);
 }

 TEST(Interpreted_Test, vector_accepts_last_allowed_record)
 {
  for (const index_t limit: {0, 1, 4, 10})
  {
   SCOPED_TRACE(limit);
   Database scalar{Record_Id(limit)};
   scalar.create_table("items");
   ASSERT_NO_THROW(scalar.insert_into(Table_Id(1), Record_Id(limit)));
   Database vector{Record_Id(limit)};
   vector.create_table("items");
   EXPECT_NO_THROW(vector.insert_vector(Table_Id(1), Record_Id(limit), 1));
   EXPECT_TRUE(vector.is_used(Table_Id(1), Record_Id(limit)));
  }
 }

 TEST(Interpreted_Test, vector_does_not_exceed_record_limit)
 {
  Database db{Record_Id(10)};
  db.create_table("items");
  EXPECT_THROW(db.insert_vector(Table_Id(1), Record_Id(9), 3), Exception);
  EXPECT_EQ(to_underlying(db.get_size(Table_Id(1))), 0);
 }

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

 ////////////////////////////////////////////////////////////////////////////
 TEST(Interpreted_Test, more_range_errors)
 ////////////////////////////////////////////////////////////////////////////
 {
  {
   joedb::Database db;
   db.create_table("person");
   EXPECT_ANY_THROW(db.insert_into(Table_Id{1}, Record_Id{-1}));
   EXPECT_NO_THROW(db.insert_into(Table_Id{1}, Record_Id{0}));
   EXPECT_ANY_THROW(db.insert_into(Table_Id{1}, Record_Id{0}));
  }
  {
   joedb::Database db;
   db.create_table("person");
   const size_t size = 123;
   EXPECT_ANY_THROW(db.insert_vector(Table_Id{1}, Record_Id{-1}, size));
   EXPECT_NO_THROW(db.insert_vector(Table_Id{1}, Record_Id{0}, size));
   EXPECT_ANY_THROW(db.insert_vector(Table_Id{1}, Record_Id{0}, size));
  }
 }
}
