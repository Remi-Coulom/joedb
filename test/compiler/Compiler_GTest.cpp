#include "db/testdb.h"
#include "db/multi_index.h"
#include "joedb/journal/Interpreted_File.h"
#include <stdexcept>

using namespace my_namespace::is_nested;

#include "gtest/gtest.h"

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, exceptions)
/////////////////////////////////////////////////////////////////////////////
{
 try
 {
  joedb::Memory_File file;
  testdb::Generic_File_Database db(file);
  db.new_city("Paris");
  db.new_city("Lille");
  db.new_city("Paris");
  db.checkpoint();
  ADD_FAILURE() << "should have thrown";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "city_by_name unique index failure: (\"Paris\") at id = 3 was already at id = 1");
 }

 try
 {
  joedb::Memory_File file;
  multi_index::Generic_File_Database db(file);
  db.new_person("Chantal", "Dupont");
  db.new_person("Rémi", "Coulom");
  db.new_person("Rémi", "Munos");
  db.new_person("Marcel", "Coulom");
  db.new_person("Albert", "Premier");
  db.new_person("Rémi", "Coulom");
  db.checkpoint();
  ADD_FAILURE() << "should have thrown";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "person_by_full_name unique index failure: (\"Rémi\", \"Coulom\") at id = 6 was already at id = 2");
 }

 try
 {
  joedb::Memory_File file;
  testdb::Generic_File_Database db(file);
  auto translation = db.new_translation();
  ((joedb::Writable *)&db)->delete_from(5, translation.get_id());
  db.checkpoint();
 }
 catch (const joedb::Exception &e)
 {
  ADD_FAILURE() << e.what();
 }

 try
 {
  joedb::Memory_File file;
  testdb::Generic_File_Database db(file);
  ((joedb::Writable *)&db)->insert_into(1, 1);
  ((joedb::Writable *)&db)->insert_into(1, 1);
  db.checkpoint();
  ADD_FAILURE() << "should have thrown";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "Duplicate insert into table city");
 }

 try
 {
  joedb::Memory_File file;
  testdb::Generic_File_Database db(file);
  ((joedb::Writable *)&db)->insert_into(5, 1);
  ((joedb::Writable *)&db)->insert_into(5, 3);
  db.checkpoint();
 }
 catch (const joedb::Exception &e)
 {
  ADD_FAILURE() << e.what();
 }

 try
 {
  joedb::Memory_File file;
  testdb::Generic_File_Database db(file);
  db.set_max_record_id(1000);
  ((joedb::Writable *)&db)->insert_into(1, 2000);
  db.checkpoint();
  ADD_FAILURE() << "should have thrown";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "insert_into: too big");
 }

 try
 {
  joedb::Memory_File file;
  testdb::Generic_File_Database db(file);
  db.set_max_record_id(1000);
  ((joedb::Writable *)&db)->insert_vector(1, 1, 2000);
  db.checkpoint();
  ADD_FAILURE() << "should have thrown";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "insert_vector: null record_id, or too big");
 }

#ifndef NDEBUG
 try
 {
  joedb::Memory_File file;
  testdb::Generic_File_Database db(file);
  auto city = db.new_city("Paris");
  EXPECT_STREQ(db.get_name(city).c_str(), "Paris");
  db.delete_city(city);
  db.checkpoint();
  db.get_name(city);
  ADD_FAILURE() << "reading a deleted row";
 }
 catch (const joedb::Assertion_Failure &a)
 {
 }
#endif

#ifndef NDEBUG
 try
 {
  joedb::Memory_File file;
  testdb::Generic_File_Database db(file);
  auto city = db.new_city("Paris");
  db.delete_city(city);
  db.checkpoint();
  db.delete_city(city);
  ADD_FAILURE() << "double delete";
 }
 catch (const joedb::Assertion_Failure &a)
 {
 }
#endif

#ifndef NDEBUG
 try
 {
  joedb::Memory_File file;
  testdb::Generic_File_Database db(file);
  auto city = db.new_city("Paris");
  db.delete_city(city);
  db.checkpoint();
  db.set_name(city, "Paris");
  ADD_FAILURE() << "update of deleted row";
 }
 catch (const joedb::Assertion_Failure &a)
 {
 }
#endif
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, iterators)
/////////////////////////////////////////////////////////////////////////////
{
 testdb::Readonly_Database db
 (
  joedb::Interpreted_File("compiler/db/test.joedbi")
 );

 {
  auto i = db.get_person_table().begin();
  EXPECT_EQ(db.get_name(*i++), "Rémi");
  EXPECT_EQ(db.get_name(*i--), "Norbert");
  EXPECT_EQ(db.get_name(*i  ), "Rémi");
  EXPECT_EQ(db.get_name(*++i), "Norbert");
  EXPECT_EQ(db.get_name(*--i), "Rémi");
 }

 {
  auto i = db.get_translation_table().begin();
  EXPECT_EQ(db.get_translation(*i++), "Hello");
  EXPECT_EQ(db.get_translation(*i--), "Bonjour");
  EXPECT_EQ(db.get_translation(*i  ), "Hello");
  EXPECT_EQ(db.get_translation(*++i), "Bonjour");
  EXPECT_EQ(db.get_translation(*--i), "Hello");
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, checkpoints)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 testdb::Generic_File_Database db(file);
 EXPECT_EQ(db.ahead_of_checkpoint(), 0);
 db.checkpoint_full_commit();
 EXPECT_EQ(db.ahead_of_checkpoint(), 0);
 db.new_city("Paris");
 EXPECT_TRUE(db.ahead_of_checkpoint() > 0);
 db.checkpoint_full_commit();
 EXPECT_EQ(db.ahead_of_checkpoint(), 0);
}
