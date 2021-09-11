#include "db/testdb.h"
#include "db/multi_index.h"
#include "db/schema_v1.h"
#include "db/schema_v2.h"
#include "joedb/journal/Interpreted_File.h"
#include "joedb/io/Interpreter_Dump_Writable.h"
#include "joedb/concurrency/Embedded_Connection.h"

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

/////////////////////////////////////////////////////////////////////////////
void schema_v2::Generic_File_Database::set_default_preferred_language_to_english
/////////////////////////////////////////////////////////////////////////////
(
 Generic_File_Database &db
)
{
 auto english = db.new_language("English", "en");
 for (auto person: db.get_person_table())
  db.set_preferred_language(person, english);
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, schema_upgrade)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;

 {
  schema_v1::Generic_File_Database db(file);

  db.new_person("Toto");
  db.new_person("Rémi");
  db.write_comment("This is a comment");
  db.write_timestamp(12345);
  db.checkpoint();
 }

 file.set_mode(joedb::Open_Mode::write_existing);

 try
 {
  schema_v2::Readonly_Database db(file);
  ADD_FAILURE() << "v2 should not open v1 readonly without upgrade\n";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "This joedb file has an old schema, and must be upgraded first.");
 }

 {
  schema_v1::Generic_File_Database db(file);
 }

 {
  schema_v2::Generic_File_Database db(file);

  const auto toto = db.get_person_table().first();
  EXPECT_EQ("Toto", db.get_name(toto));
  const auto english = db.get_language_table().first();
  EXPECT_EQ("English", db.get_name(english));
  EXPECT_EQ(english, db.get_preferred_language(toto));
 }

 {
  schema_v2::Generic_File_Database db(file);
 }

 try
 {
  schema_v1::Generic_File_Database db(file);
  ADD_FAILURE() << "v1 code should not open v2 files\n";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "Trying to open a file with incompatible schema");
 }

 try
 {
  schema_v1::Readonly_Database db(file);
  ADD_FAILURE() << "v1 code should not open v2 files\n";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "Trying to open a file with incompatible schema");
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, client)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File server_file;
 joedb::Embedded_Connection connection(server_file);

 joedb::Memory_File client_v1_file;
 schema_v1::Client client_v1(connection, client_v1_file);

 joedb::Memory_File client_v1bis_file;
 schema_v1::Client client_v1bis(connection, client_v1bis_file);

 client_v1.transaction([](schema_v1::Generic_File_Database &db)
 {
  db.new_person("Toto");
 });

 joedb::Memory_File client_v2_file;
 schema_v2::Client client_v2(connection, client_v2_file);

 client_v2.transaction([](schema_v2::Generic_File_Database &db)
 {
  db.new_language("French", "fr");
 });

 try
 {
  client_v1.pull();
  ADD_FAILURE() << "client_v1 should not be able to pull new schema\n";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "Can't upgrade schema during pull");
 }

 try
 {
  client_v1bis.transaction([](schema_v1::Generic_File_Database &db)
  {
  });
  ADD_FAILURE() <<  "client_v1 should not be able to pull new schema\n";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "Can't upgrade schema during pull");
 }
}
