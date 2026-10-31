#include "joedb/interpreted/Database.h"
#include "gtest/gtest.h"
#include <limits>

namespace joedb
{
 TEST(Interpreted_Test, oversized_vector_delete_preserves_records)
 {
  Database db;
  db.create_table("items");
  const Table_Id table{1};
  db.insert_vector(table, Record_Id{0}, 3);

  for (int pass = 0; pass < 2; ++pass)
  {
   EXPECT_THROW(db.delete_vector(table, Record_Id{1},
                std::numeric_limits<size_t>::max()), Exception);
   EXPECT_THROW(db.delete_vector(table, Record_Id{1},
                size_t(std::numeric_limits<index_t>::max())), Exception);
   EXPECT_TRUE(db.get_freedom(table).is_used_vector(Record_Id{1}, 2));
   if (pass == 0)
    db.delete_from(table, Record_Id{0});
  }

  EXPECT_NO_THROW(db.delete_vector(table, Record_Id{1}, 2));
  EXPECT_EQ(db.get_freedom(table).get_used_count(), Record_Id{0});
 }

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
