#include "db/test/Client.h"
#include "db/multi_index/Writable_Database.h"
#include "db/schema_v1/Client.h"
#include "db/schema_v1/Readonly_Database.h"
#include "db/schema_v2/Client.h"
#include "db/schema_v2/Readonly_Database.h"
#include "db/test/Readonly_Database.h"
#include "db/vector_test/Writable_Database.h"
#include "db/vector_test/Readonly_Database.h"
#include "translation.h"
#include "joedb/journal/Readonly_Interpreted_File.h"
#include "joedb/concurrency/File_Connection.h"
#include "joedb/interpreted/Database.h"
#include "joedb/journal/Upgradable_File.h"
#include "joedb/journal/Readonly_Memory_File.h"

using namespace my_namespace::is_nested;

#include "gtest/gtest.h"

/////////////////////////////////////////////////////////////////////////////
static const std::string &get_translation
/////////////////////////////////////////////////////////////////////////////
(
 const test::Database &db,
 test::id_of_string_id string_id,
 test::id_of_language language
)
{
 auto translation = db.find_translation_by_ids(string_id, language);

 if (translation.is_null())
  translation = db.find_translation_by_ids(string_id, test::language::en);

 if (translation.is_not_null())
  return db.get_translation(translation);

 const static std::string error("Translation error!");
 return error;
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, id_test)
/////////////////////////////////////////////////////////////////////////////
{
 test::id_of_person null_person(0);
 test::id_of_person non_null_person(1234);

 EXPECT_TRUE(null_person.is_null());
 EXPECT_FALSE(null_person.is_not_null());
 EXPECT_FALSE(non_null_person.is_null());
 EXPECT_TRUE(non_null_person.is_not_null());
 EXPECT_EQ(null_person.get_id(), 0UL);
 EXPECT_EQ(non_null_person.get_id(), 1234UL);
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, file_test)
/////////////////////////////////////////////////////////////////////////////
{
 //
 // First, try to open the database
 //
 joedb::Memory_File file;
 joedb::Readonly_Interpreted_File("compiler/db/test.joedbi").copy_to(file);
 test::Writable_Database db(file);

 //
 // Check table sizes
 //
 EXPECT_EQ(db.get_city_table().get_size(), 2ULL);
 EXPECT_EQ(db.get_person_table().get_size(), 3ULL);

 //
 // Clear Persons and cities
 //
 db.clear_city_table();
 db.clear_person_table();

 //
 // Print table sizes
 //
 EXPECT_EQ(db.get_city_table().get_size(), 0ULL);
 EXPECT_EQ(db.get_person_table().get_size(), 0ULL);

 //
 // Insert some
 //
 db.new_city("Barcelona");
 auto New_York = db.new_city("New York");
 db.new_city("Paris");
 db.new_city("Tokyo");
 db.new_person("Toto", New_York);
 db.new_person("Évariste", db.null_city());
 db.new_person("Catherine", db.null_city());

 EXPECT_EQ(db.get_person_table().get_size(), 3ULL);
 EXPECT_EQ(db.get_city_table().get_size(), 4ULL);

 EXPECT_EQ(db.get_city_table().first().get_id(), 1ULL);
 EXPECT_EQ(db.get_city_table().last().get_id(), 4ULL);
 EXPECT_EQ(db.get_name(db.get_city_table().first()), "Barcelona");
 EXPECT_EQ(db.get_name(db.get_city_table().last()), "Tokyo");

 //
 // Test unique index constraint
 //
 try
 {
  db.set_name(New_York, "Paris");
  ADD_FAILURE() << "duplicate Paris";
 }
 catch(const std::runtime_error &e)
 {
  EXPECT_STREQ(e.what(), "my_namespace::is_nested::test: city_by_name unique index failure: (\"Paris\") at id = 2 was already at id = 3");
 }

 // This would be nice to have
 // EXPECT_EQ(db.get_name(New_York), "New York");

 //
 // Validity + get_at
 //
 EXPECT_EQ(db.get_name(db.find_city_by_name("Paris")), "Paris");
 db.delete_city(db.find_city_by_name("Paris"));

 EXPECT_FALSE(db.get_city_table().is_valid_at(0));
 EXPECT_TRUE (db.get_city_table().is_valid_at(1));
 EXPECT_TRUE (db.get_city_table().is_valid_at(2));
 EXPECT_FALSE(db.get_city_table().is_valid_at(3));
 EXPECT_TRUE (db.get_city_table().is_valid_at(4));
 EXPECT_FALSE(db.get_city_table().is_valid_at(5));
 EXPECT_FALSE(db.get_city_table().is_valid_at(6));

 EXPECT_EQ(db.get_name(test::container_of_city::get_at(1)), "Barcelona");
 EXPECT_EQ(db.get_name(test::container_of_city::get_at(4)), "Tokyo");

 //
 // Loop over not-unique index
 //
 db.new_person("Toto", db.find_city_by_name("Tokyo"));
 db.new_person("Toto", db.find_city_by_name("Barcelona"));

 {
  int count = 0;
  for (auto toto: db.find_person_by_name("Toto"))
  {
   EXPECT_EQ(db.get_name(toto), "Toto");
   count++;
  }
  EXPECT_EQ(count, 3);
 }

 EXPECT_TRUE(db.find_person_by_name("Totox").empty());

 {
  const auto range = db.find_person_by_name("Toto");
  int count = 0;
  for (auto x = range.begin(); x != range.end(); ++x)
   count++;
  EXPECT_TRUE(range.begin() != range.end());
  EXPECT_FALSE(range.empty());
  EXPECT_EQ(count, 3);
  EXPECT_EQ(range.size(), 3ULL);
 }

 //
 // Standard find algorithm
 //
 {
  auto i = std::find_if
  (
   db.get_person_table().begin(),
   db.get_person_table().end(),
   [&](test::id_of_person person)
   {
    return db.get_name(person) == "Catherine";
   }
  );
  EXPECT_TRUE(i != db.get_person_table().end());
 }

 //
 // Isn't it simpler with a plain loop?
 //
 {
  bool found = false;
  for (auto person: db.get_person_table())
   if (db.get_name(person) == "Catherine")
   {
    found = true;
    break;
   }
  EXPECT_TRUE(found);
 }

 //
 // Translation test
 //
 EXPECT_EQ
 (
  get_translation
  (
   db,
   test::string_id::how_are_you,
   test::language::fr
  ),
  "Comment allez-vous?"
 );

 //
 // Sorting
 //
 {
  db.new_person("Zoé", db.null_city());
  db.new_person("Albert", db.null_city());
  auto by_name = [&](test::id_of_person p_1, test::id_of_person p_2)
  {
   return db.get_name(p_1) < db.get_name(p_2);
  };

  std::vector<std::string> v;
  for (auto person: db.sorted_person(by_name))
   v.emplace_back(db.get_name(person));

  EXPECT_EQ(v.front(), "Albert");
  EXPECT_EQ(v.back(), "Évariste");
 }

 //
 // Sorting with index
 //
 {
  std::vector<std::string> v;
  for (const auto &x: db.get_index_of_person_by_name())
   v.emplace_back(db.get_name(x.second));

  EXPECT_EQ(v.front(), "Albert");
  EXPECT_EQ(v.back(), "Évariste");
 }

 //
 // Inserting a vector with a unique index
 //
 {
  auto v = db.new_vector_of_city(2);
  db.set_name(v[0], "Washington");
  db.set_name(v[1], "Beijing");

  std::vector<std::string> vs;

  for
  (
   auto x = db.get_index_of_city_by_name().rbegin();
   x != db.get_index_of_city_by_name().rend();
   ++x
  )
  {
   vs.emplace_back(db.get_name(x->second));
  }

  EXPECT_EQ(vs.front(), "Washington");
  EXPECT_EQ(vs.back(), "Barcelona");
 }

 //
 // New element in table with no field
 //
 {
  auto x = db.new_table_with_no_field();
  EXPECT_EQ(x.get_id(), 1ULL);
 }

 //
 // Comparison operators
 //
 {
  auto W = db.find_city_by_name("Washington");
  auto B = db.find_city_by_name("Beijing");

  EXPECT_FALSE(W == B);
  EXPECT_TRUE (W != B);
  EXPECT_TRUE (W <  B);
  EXPECT_TRUE (W <= B);
  EXPECT_FALSE(W >  B);
  EXPECT_FALSE(W >= B);
  EXPECT_TRUE (B == B);
  EXPECT_FALSE(B != B);
  EXPECT_FALSE(B <  B);
  EXPECT_TRUE (B <= B);
  EXPECT_FALSE(B >  B);
  EXPECT_TRUE (B >= B);
 }

 db.checkpoint();
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, exceptions)
/////////////////////////////////////////////////////////////////////////////
{
 try
 {
  joedb::Memory_File file;
  test::Writable_Database db(file);
  db.new_city("Paris");
  db.new_city("Lille");
  db.new_city("Paris");
  db.checkpoint();
  ADD_FAILURE() << "should have thrown";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "my_namespace::is_nested::test: city_by_name unique index failure: (\"Paris\") at id = 3 was already at id = 1");
 }

 try
 {
  joedb::Memory_File file;
  multi_index::Writable_Database db(file);
  db.new_person("Chantal", "Dupont", db.null_city());
  db.new_person("Rémi", "Coulom", db.null_city());
  db.new_person("Rémi", "Munos", db.null_city());
  db.new_person("Marcel", "Coulom", db.null_city());
  db.new_person("Albert", "Premier", db.null_city());
  db.new_person("Rémi", "Coulom", db.null_city());
  db.checkpoint();
  ADD_FAILURE() << "should have thrown";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "multi_index: person_by_full_name unique index failure: (\"Rémi\", \"Coulom\") at id = 6 was already at id = 2");
 }

 try
 {
  joedb::Memory_File file;
  test::Writable_Database db(file);
  auto translation = db.new_translation();
  ((joedb::Writable *)&db)->delete_from(joedb::Table_Id(5), translation.get_record_id());
  db.checkpoint();
 }
 catch (const joedb::Exception &e)
 {
  ADD_FAILURE() << e.what();
 }

 try
 {
  joedb::Memory_File file;
  test::Writable_Database db(file);
  ((joedb::Writable *)&db)->insert_into(joedb::Table_Id(1), joedb::Record_Id(1));
  ((joedb::Writable *)&db)->insert_into(joedb::Table_Id(1), joedb::Record_Id(1));
  db.checkpoint();
  ADD_FAILURE() << "should have thrown";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "my_namespace::is_nested::test: Duplicate insert into table city");
 }

 try
 {
  joedb::Memory_File file;
  test::Writable_Database db(file);
  ((joedb::Writable *)&db)->insert_into(joedb::Table_Id(5), joedb::Record_Id(1));
  ((joedb::Writable *)&db)->insert_into(joedb::Table_Id(5), joedb::Record_Id(3));
  db.checkpoint();
 }
 catch (const joedb::Exception &e)
 {
  ADD_FAILURE() << e.what();
 }

 try
 {
  joedb::Memory_File file;
  test::Writable_Database db(file);
  db.set_max_record_id(1000);
  ((joedb::Writable *)&db)->insert_into(joedb::Table_Id(1), joedb::Record_Id(2000));
  db.checkpoint();
  ADD_FAILURE() << "should have thrown";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "my_namespace::is_nested::test: insert_into: too big");
 }

 try
 {
  joedb::Memory_File file;
  test::Writable_Database db(file);
  db.set_max_record_id(1000);
  ((joedb::Writable *)&db)->insert_vector(joedb::Table_Id(1), joedb::Record_Id(1), 2000);
  db.checkpoint();
  ADD_FAILURE() << "should have thrown";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "my_namespace::is_nested::test: insert_vector: null record_id, or too big");
 }

#ifndef NDEBUG
 try
 {
  joedb::Memory_File file;
  test::Writable_Database db(file);
  auto city = db.new_city("Paris");
  EXPECT_STREQ(db.get_name(city).c_str(), "Paris");
  db.delete_city(city);
  db.checkpoint();
  db.get_name(city);
  ADD_FAILURE() << "reading a deleted row";
 }
 catch (const std::exception &)
 {
 }
#endif

#ifndef NDEBUG
 try
 {
  joedb::Memory_File file;
  test::Writable_Database db(file);
  auto city = db.new_city("Paris");
  db.delete_city(city);
  db.checkpoint();
  db.delete_city(city);
  ADD_FAILURE() << "double delete";
 }
 catch (const std::exception &)
 {
 }
#endif

#ifndef NDEBUG
 try
 {
  joedb::Memory_File file;
  test::Writable_Database db(file);
  auto city = db.new_city("Paris");
  db.delete_city(city);
  db.checkpoint();
  db.set_name(city, "Paris");
  ADD_FAILURE() << "update of deleted row";
 }
 catch (const std::exception &)
 {
 }
#endif
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, iterators)
/////////////////////////////////////////////////////////////////////////////
{
 test::Readonly_Database db
 (
  joedb::Readonly_Interpreted_File("compiler/db/test.joedbi")
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
 test::Writable_Database db(file);
 EXPECT_EQ(db.ahead_of_checkpoint(), 0);
 db.checkpoint_full_commit();
 EXPECT_EQ(db.ahead_of_checkpoint(), 0);
 db.new_city("Paris");
 EXPECT_TRUE(db.ahead_of_checkpoint() > 0);
 db.checkpoint_full_commit();
 EXPECT_EQ(db.ahead_of_checkpoint(), 0);
}

/////////////////////////////////////////////////////////////////////////////
void schema_v2::Writable_Database::set_default_preferred_language_to_english
/////////////////////////////////////////////////////////////////////////////
(
 Writable_Database &db
)
{
 const auto english = db.new_language("English", "en");
 for (auto person: db.get_person_table())
  db.set_preferred_language(person, english);
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, schema_upgrade)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;

 {
  schema_v1::Writable_Database db(file);

  db.new_person("Toto");
  db.new_person("Rémi");
  db.write_comment("This is a comment");
  db.write_timestamp(12345);
  db.checkpoint();
 }

 try
 {
  schema_v2::Readonly_Database db(file);
  ADD_FAILURE() << "v2 should not open v1 readonly without upgrade\n";
 }
 catch (const joedb::Out_Of_Date &e)
 {
  EXPECT_STREQ(e.what(), "schema_v2: Schema is out of date. Can't upgrade a read-only database.");
 }

 {
  schema_v1::Writable_Database db(file);
 }

 {
  joedb::Upgradable_File<joedb::Readonly_Memory_File> upgradable_file
  (
   file.get_data()
  );

  schema_v2::Writable_Database db(upgradable_file);

  const auto toto = db.get_person_table().first();
  EXPECT_EQ("Toto", db.get_name(toto));
  const auto english = db.get_language_table().first();
  EXPECT_EQ("English", db.get_name(english));
  EXPECT_EQ(english, db.get_preferred_language(toto));
 }

 {
  schema_v2::Writable_Database db(file);

  const auto toto = db.get_person_table().first();
  EXPECT_EQ("Toto", db.get_name(toto));
  const auto english = db.get_language_table().first();
  EXPECT_EQ("English", db.get_name(english));
  EXPECT_EQ(english, db.get_preferred_language(toto));
 }

 {
  schema_v2::Writable_Database db(file);
 }

 try
 {
  schema_v1::Writable_Database db(file);
  ADD_FAILURE() << "v1 code should not open v2 files\n";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "schema_v1: Trying to open a file with incompatible schema");
 }

 try
 {
  schema_v1::Readonly_Database db(file);
  ADD_FAILURE() << "v1 code should not open v2 files\n";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "schema_v1: Trying to open a file with incompatible schema");
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, client)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File server_file;

 joedb::File_Connection connection(server_file);

 joedb::Memory_File client_v1_file;
 schema_v1::Client client_v1(client_v1_file, connection);

 joedb::Memory_File client_v1bis_file;
 schema_v1::Client client_v1bis(client_v1bis_file, connection);

 client_v1.transaction([](schema_v1::Writable_Database &db)
 {
  db.new_person("Toto");
 });

 joedb::Memory_File client_v2_file;
 schema_v2::Client client_v2(client_v2_file, connection);

 client_v2.transaction([](schema_v2::Writable_Database &db)
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
  EXPECT_STREQ(e.what(), "schema_v1: Can't upgrade schema during pull");
 }

 try
 {
  client_v1bis.transaction([](schema_v1::Writable_Database &db)
  {
  });
  ADD_FAILURE() <<  "client_v1 should not be able to pull new schema\n";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "schema_v1: Can't upgrade schema during pull");
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, client_push)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File client_file;

 {
  test::Writable_Database db(client_file);
  db.new_person("Rémi", db.null_city());
  db.checkpoint();
 }

 joedb::Memory_File server_file;
 joedb::File_Connection connection(server_file);

 {
  test::Client client(client_file, connection);
  EXPECT_TRUE(client.get_checkpoint_difference() == 0);
 }

 test::Writable_Database db(server_file);
 EXPECT_FALSE(db.find_person_by_name("Rémi").empty());
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, client_hash_error)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File client_file;

 {
  test::Writable_Database db(client_file);
  db.new_person("Rémi", db.null_city());
  db.checkpoint();
 }

 joedb::Memory_File server_file;

 {
  test::Writable_Database db(server_file);
  db.new_person("X", db.null_city());
  db.checkpoint();
 }

 joedb::File_Connection connection(server_file);

 try
 {
  test::Client client(client_file, connection);
  ADD_FAILURE() << "Should have thrown\n";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "Content mismatch. The file and the connection have diverged, and cannot be synced by pulling or pushing.");
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, vector)
/////////////////////////////////////////////////////////////////////////////
{
 //
 // Freedom_keeper version
 //
 {
  joedb::Memory_File file;

  const size_t n = 5;
  test::id_of_float v;

  {
   test::Writable_Database db(file);
   v = db.new_vector_of_float(n);
   for (size_t i = 0; i < n; i++)
    db.set_value(v[i], 0.1f * float(i));
   db.checkpoint();
  }

  {
   test::Readonly_Database db(file);
   for (size_t i = 0; i < n; i++)
    EXPECT_FLOAT_EQ(db.get_value(v[i]), 0.1f * float(i));
  }
 }

 //
 // Allocating empty vectors works
 //
 {
  joedb::Memory_File file;

  {
   vector_test::Writable_Database db(file);
   db.new_vector_of_point(0);
   db.checkpoint();
  }
  {
   vector_test::Readonly_Database db(file);
   EXPECT_EQ(db.get_point_table().get_size(), 0ULL);
  }
 }

 //
 // Vector storage
 //
 {
  const size_t n = 5;
  joedb::Memory_File file;

  {
   vector_test::Writable_Database db(file);
   auto v = db.new_vector_of_point(n);
   for (size_t i = 0; i < n; i++)
   {
    db.set_x(v[i], 0.1f * float(i));
    db.set_y(v[i], 1.234f);
   }
   db.checkpoint();

   EXPECT_EQ(db.get_point_table().first().get_id(), 1ULL);
   EXPECT_EQ(db.get_point_table().last().get_id(), 5ULL);
  }

  {
   vector_test::Writable_Database db(file);
   auto v = db.new_vector_of_point(n);

   db.update_vector_of_x(v, n, [&](joedb::Span<float> x)
   {
    db.update_vector_of_y(v, n, [&](joedb::Span<float> y)
    {
     for (size_t i = 0; i < n; i++)
     {
      x[i] = 0.2f * float(i);
      y[i] = 5.678f;
     }
    });
   });

   db.checkpoint();
  }

  {
   vector_test::Readonly_Database db(file);
   auto v = db.get_point_table().first();
   for (size_t i = 0; i < n; i++)
   {
    EXPECT_FLOAT_EQ(db.get_x(v[i]), 0.1f * float(i));
    EXPECT_FLOAT_EQ(db.get_y(v[i]), 1.234f);
   }
   for (size_t i = n; i < 2 * n; i++)
   {
    EXPECT_FLOAT_EQ(db.get_x(v[i]), 0.2f * float(i - n));
    EXPECT_FLOAT_EQ(db.get_y(v[i]), 5.678f);
   }
  }
 }

 try
 {
  joedb::Memory_File file;
  joedb::Readonly_Interpreted_File("compiler/vector_hole.joedbi").copy_to(file);
  vector_test::Writable_Database db(file);
 }
 catch (const joedb::Exception &e)
 {
  ADD_FAILURE() << e.what();
 }

 try
 {
  joedb::Memory_File file;
  joedb::Readonly_Interpreted_File("compiler/vector_hole_by_vector_insert.joedbi").copy_to(file);

  {
   vector_test::Writable_Database db(file);
   db.set_x(db.get_point_table().first(), 1.234f);
   db.checkpoint();
  }
  {
   joedb::Readonly_Journal journal(file);
   joedb::Database database;
   journal.replay_log(database);
  }
 }
 catch (const joedb::Exception &e)
 {
  ADD_FAILURE() << e.what();
 }
 catch (const joedb::Assertion_Failure &e)
 {
  ADD_FAILURE() << e.what();
 }

 try
 {
  joedb::Memory_File file;
  joedb::Readonly_Interpreted_File("compiler/vector_delete.joedbi").copy_to(file);
  vector_test::Writable_Database db(file);
 }
 catch (const joedb::Exception &e)
 {
  ADD_FAILURE() << e.what();
 }

 //
 // Vector of strings with a unique index
 //
 {
  joedb::Memory_File file;
  vector_test::Writable_Database db(file);

  {
   constexpr int n = 3;
   auto v = db.new_vector_of_person(n);
   db.update_vector_of_name(v, n, [](joedb::Span<std::string> name)
   {
    name[0] = "Rémi";
    name[1] = "Paul";
    name[2] = "Liza";
   });
  }

  {
   auto remi = db.find_person_by_name("Rémi");
   if (remi.is_not_null())
    EXPECT_EQ(db.get_name(remi), "Rémi");
   else
    ADD_FAILURE() << "Rémi not found";
  }

  {
   constexpr int n = 3;
   auto v = db.get_person_table().first();
   db.update_vector_of_name(v, n, [](joedb::Span<std::string> name)
   {
    name[0] = "Joe";
    name[1] = "Max";
    name[2] = "Liz";
   });
  }

  EXPECT_TRUE(db.find_person_by_name("Rémi").is_null());

  {
   auto joe = db.find_person_by_name("Joe");
   if (joe.is_not_null())
    EXPECT_EQ(db.get_name(joe), "Joe");
   else
    ADD_FAILURE() << "Joe not found";
  }

  db.checkpoint();
 }

 //
 // timestamp
 //
 {
  joedb::Memory_File file;
  vector_test::Writable_Database db(file);
  db.write_timestamp();
  db.write_comment("This was a timestamp.");
  db.checkpoint();
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, index_iteration)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 test::Writable_Database db(file);

 const auto tokyo = db.new_city("Tokyo");
 const auto lille = db.new_city("Lille");
 const auto abidjan = db.new_city("Abidjan");
 const auto paris = db.new_city("Paris");

 db.checkpoint_no_commit();

 EXPECT_EQ(db.next_city_by_name(lille), paris);
 EXPECT_EQ(db.next_city_by_name(paris), tokyo);
 EXPECT_EQ(db.previous_city_by_name(lille), abidjan);
 EXPECT_EQ(db.previous_city_by_name(abidjan), db.null_city());

 EXPECT_EQ(db.next(tokyo), lille);
 EXPECT_EQ(db.previous(lille), tokyo);
 EXPECT_EQ(db.next(paris), db.get_city_table().get_end());
}

#ifdef JOEDB_FILE_IS_LOCKABLE
/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, shared_local_file)
/////////////////////////////////////////////////////////////////////////////
{
 const char * const file_name = "compiler_test.joedb";
 std::remove(file_name);

 {
  test::File_Client client(file_name);
  EXPECT_EQ(client.get_database().get_city_table().get_size(), 0ULL);
  client.pull();
 }

 std::remove(file_name);
}
#endif

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, delete_vector)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 test::Writable_Database db(file);

 const auto v = db.new_vector_of_city(100);

 const size_t first = 20;
 const size_t size = 40;

 db.delete_vector_of_city(v[first], size);
 db.default_checkpoint();

 for (size_t i = 0; i < 100; i++)
 {
  const auto city = v[i];
  if (city >= v[first] && city < v[first + size])
   EXPECT_FALSE(db.is_valid(city));
  else
   EXPECT_TRUE(db.is_valid(city));
 }
}
