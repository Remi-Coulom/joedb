#include "db/test/Client.h"
#include "db/default_values/Writable_Database.h"
#include "db/test/Readonly_Database.h"
#include "db/test/Memory_Database.h"
#include "db/test/File_Client.h"
#include "db/multi_index/Memory_Database.h"
#include "db/multi_index/Readonly_Database.h"
#include "db/schema_v1/Client.h"
#include "db/schema_v1/Readonly_Database.h"
#include "db/schema_v2/Client.h"
#include "db/schema_v2/Readonly_Database.h"
#if __cplusplus >= 202002L
 #include "db/unordered_index/Writable_Database.h"
#endif
#include "db/vector_test/Writable_Database.h"
#include "db/vector_test/Readonly_Database.h"
#include "db/vector_test/Memory_Database.h"
#include "translation.h"
#include "joedb/journal/Interpreted_File.h"
#include "joedb/concurrency/File_Connection.h"
#include "joedb/interpreted/Database.h"
#include "joedb/journal/Upgradable_File.h"
#include "joedb/journal/Readonly_Memory_File.h"
#include "joedb/compiler/Compiler_Options.h"
#include "joedb/compiler/Compiler_Options_io.h"

using namespace my_namespace::is_nested;

#include "gtest/gtest.h"

#include <unordered_map>
#include <array>
#include <map>
#include <set>
#include <random>
#include <filesystem>
#include <fstream>
#include <sstream>
#if defined(__linux__) && GTEST_HAS_DEATH_TEST
#include <unistd.h>
#include <cstdlib>

namespace
{
 class Compiler_Command_DeathTest: public testing::Test
 {
  protected:
   std::filesystem::path directory;
   std::string compiler;
   void SetUp() override
   {
    compiler = (std::filesystem::read_symlink("/proc/self/exe").parent_path() / "joedbc").string();
    ASSERT_TRUE(std::filesystem::exists(compiler));
    std::string pattern = (std::filesystem::temp_directory_path() / "joedb-compiler-XXXXXX").string();
    const char *created = ::mkdtemp(pattern.data());
    ASSERT_NE(created, nullptr);
    directory = created;
    std::ofstream(directory / "schema.joedbi") << "create_table items\nadd_field items value int32\n";
    options("");
   }
   void options(const std::string &text)
   {
    std::ofstream(directory / "schema.joedbc") << "namespace generated\n" << text;
   }
   void expect_failure(const char *message)
   {
    EXPECT_EXIT
    ({
     if (::chdir(directory.c_str()) != 0)
      std::_Exit(127);
     ::execl(compiler.c_str(), compiler.c_str(), "schema", static_cast<char *>(nullptr));
     std::_Exit(127);
    }, testing::ExitedWithCode(1), message);
   }
   void TearDown() override
   {
    if (!directory.empty())
     std::filesystem::remove_all(directory);
   }
 };

 TEST_F(Compiler_Command_DeathTest, help_and_version)
 {
  EXPECT_EXIT
  ({
   ::execl(compiler.c_str(), compiler.c_str(), static_cast<char *>(nullptr));
   std::_Exit(127);
  }, testing::ExitedWithCode(1), "usage:");
  EXPECT_EXIT
  ({
   ::execl(compiler.c_str(), compiler.c_str(), "--version", static_cast<char *>(nullptr));
   std::_Exit(127);
  }, testing::ExitedWithCode(0), "");
 }
 TEST_F(Compiler_Command_DeathTest, unknown_option)
 {
  options("unknown_option\n");
  expect_failure("unknown command: unknown_option");
 }
 TEST_F(Compiler_Command_DeathTest, unknown_index_field)
 {
  options("create_index lookup items missing\n");
  expect_failure("Field not found: missing");
 }
 TEST_F(Compiler_Command_DeathTest, invalid_index_identifier)
 {
  options("create_index bad-name items value\n");
  expect_failure("Invalid index identifier: bad-name");
 }
 TEST_F(Compiler_Command_DeathTest, invalid_custom_identifier)
 {
  std::ofstream(directory / "schema.joedbi") << "custom bad-name\n";
  expect_failure("custom: invalid identifier");
 }
 TEST_F(Compiler_Command_DeathTest, deprecated_storage_option_warns)
 {
  options("set_table_storage\n");
  EXPECT_EXIT
  ({
   if (::chdir(directory.c_str()) != 0)
    std::_Exit(127);
   ::execl(compiler.c_str(), compiler.c_str(), "schema", static_cast<char *>(nullptr));
   std::_Exit(127);
  }, testing::ExitedWithCode(0), "set_table_storage is deprecated");
  EXPECT_TRUE(std::filesystem::exists(directory / "generated" / "Database.h"));
 }
}
#endif

TEST(Compiler_Command, index_requires_fields)
{
 for (const char *command: {"create_index", "create_unique_index",
                           "create_unordered_index", "create_unordered_unique_index"})
 {
  SCOPED_TRACE(command);
  joedb::Compiler_Options options;
  options.db.create_table("items");
  options.db.add_field(joedb::Table_Id(1), "value", joedb::Type::int32());
  std::istringstream input(std::string(command) + " lookup items\n");
  std::ostringstream errors;
  EXPECT_THROW(joedb::parse_compiler_options(input, options, errors), joedb::Exception);
  EXPECT_TRUE(options.get_indices().empty());
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, deterministic_multi_index_model)
/////////////////////////////////////////////////////////////////////////////
{
 multi_index::Memory_Database db;
 const std::array<multi_index::id_of_city, 3> cities =
  {db.new_city(), db.new_city(), db.new_city()};
 struct Row
 {
  std::string first;
  std::string last;
  multi_index::id_of_city city;
 };
 std::map<multi_index::id_of_person, Row> model;
 const std::array<std::string, 5> names =
  {"", "Smith", "Coulom", std::string("A\0B", 3), "Dupont"};
 std::mt19937 random(0x51de);

 const auto check = [&](const auto &database)
 {
  EXPECT_EQ(database.get_person_table().get_size(), model.size());
  for (const auto &entry: model)
  {
   const auto id = entry.first;
   const Row &row = entry.second;
   EXPECT_EQ(database.get_first_name(id), row.first);
   EXPECT_EQ(database.get_last_name(id), row.last);
   EXPECT_EQ(database.get_city(id), row.city);
   EXPECT_EQ(database.find_person_by_full_name(row.first, row.last), id);
  }
  for (const auto &name: names)
  {
   std::set<multi_index::id_of_person> expected;
   for (const auto &entry: model)
    if (entry.second.last == name)
     expected.insert(entry.first);
   std::set<multi_index::id_of_person> actual;
   for (const auto id: database.find_person_by_name(name))
    EXPECT_TRUE(actual.insert(id).second);
   EXPECT_EQ(actual, expected);
  }
  for (const auto city: cities)
  {
   std::set<multi_index::id_of_person> expected;
   for (const auto &entry: model)
    if (entry.second.city == city)
     expected.insert(entry.first);
   std::set<multi_index::id_of_person> actual;
   for (const auto id: database.find_person_by_city(city))
    EXPECT_TRUE(actual.insert(id).second);
   EXPECT_EQ(actual, expected);
  }
 };

 for (size_t step = 0; step < 1000; ++step)
 {
  SCOPED_TRACE(step);
  const auto operation = random() % 7;
  if (model.empty() || (operation < 2 && model.size() < 40))
  {
   const Row row{"first" + std::to_string(step), names[random() % names.size()],
                 cities[random() % cities.size()]};
   const auto id = db.new_person(row.first, row.last, row.city);
   ASSERT_TRUE(model.emplace(id, row).second);
  }
  else
  {
   auto entry = model.begin();
   std::advance(entry, random() % model.size());
   if (operation == 2)
   {
    db.delete_person(entry->first);
    model.erase(entry);
   }
   else if (operation == 3)
   {
    entry->second.first = "renamed" + std::to_string(step);
    db.set_first_name(entry->first, entry->second.first);
    // Multi-column indexes are refreshed by updating their last column.
    db.set_last_name(entry->first, entry->second.last);
   }
   else if (operation == 4)
   {
    entry->second.last = names[random() % names.size()];
    db.set_last_name(entry->first, entry->second.last);
   }
   else if (operation == 5)
   {
    entry->second.city = cities[random() % cities.size()];
    db.set_city(entry->first, entry->second.city);
   }
   else if (step % 31 == 0)
   {
    db.clear_person_table();
    model.clear();
   }
  }
  db.soft_checkpoint();
  check(db);
  if (testing::Test::HasFailure())
   return;
  if (step % 37 == 0)
  {
   multi_index::Readonly_Database replayed(db);
   check(replayed);
   if (testing::Test::HasFailure())
    return;
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, multi_index_recovers_after_duplicate)
/////////////////////////////////////////////////////////////////////////////
{
 multi_index::Memory_Database db;
 const auto first = db.new_person("Jane", "Smith", db.null_city());
 const auto second = db.new_person("Jane", "Jones", db.null_city());
 EXPECT_THROW(db.set_last_name(second, "Smith"), joedb::Exception);
 // Failed updates remain in memory: repair them before further use.
 ASSERT_NO_THROW(db.set_last_name(second, "Brown"));
 db.soft_checkpoint();
 EXPECT_EQ(db.find_person_by_full_name("Jane", "Smith"), first);
 EXPECT_EQ(db.find_person_by_full_name("Jane", "Brown"), second);
 EXPECT_TRUE(db.find_person_by_full_name("Jane", "Jones").is_null());
 multi_index::Readonly_Database replayed(db);
 EXPECT_EQ(replayed.get_last_name(second), "Brown");
 EXPECT_EQ(replayed.find_person_by_full_name("Jane", "Smith"), first);
 EXPECT_EQ(replayed.find_person_by_full_name("Jane", "Brown"), second);
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, vector_permutation_of_unique_index)
/////////////////////////////////////////////////////////////////////////////
{
 multi_index::Memory_Database db;
 const auto people = db.new_vector_of_person(3);
 db.update_vector_of_first_name(people, 3, [](auto values)
 {
  for (auto &value: values)
   value = "Jane";
 });
 const std::array<std::string, 3> names = {"Smith", "Jones", "Brown"};
 db.update_vector_of_last_name(people, 3, [&](auto values)
 {
  std::copy(names.begin(), names.end(), values.begin());
 });
 for (size_t rotation = 0; rotation < 12; ++rotation)
 {
  db.update_vector_of_last_name(people, 3, [&](auto values)
  {
   for (size_t i = 0; i < 3; ++i)
    values[i] = names[(i + rotation) % names.size()];
  });
  db.soft_checkpoint();
  multi_index::Readonly_Database replayed(db);
  for (size_t i = 0; i < 3; ++i)
  {
   const auto &name = names[(i + rotation) % names.size()];
   EXPECT_EQ(db.find_person_by_full_name("Jane", name), people[i]);
   EXPECT_EQ(replayed.find_person_by_full_name("Jane", name), people[i]);
  }
 }
 db.delete_vector_of_person(people, 3);
 EXPECT_TRUE(db.get_index_of_person_by_full_name().empty());
 EXPECT_TRUE(db.get_index_of_person_by_name().empty());
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, default_values)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 {
  joedb::Writable_Journal journal(file);
  journal.create_table("target");
  journal.create_table("item");
  journal.insert_into(joedb::Table_Id{2}, joedb::Record_Id{0});
  journal.soft_checkpoint();
 }

 default_values::Writable_Database db(file);
 const auto item = db.get_item_table().first();
 EXPECT_EQ(db.get_text(item), std::string("a\0b", 3));
 EXPECT_EQ(db.get_single(item), 1.23456789f);
 EXPECT_EQ(db.get_double_precision(item), 1.2345678901234567);
 EXPECT_TRUE(db.get_data(item).is_null());
 EXPECT_TRUE(db.get_reference(item).is_null());
 EXPECT_EQ(db.get_integer(item), 42);
 EXPECT_TRUE(db.get_boolean(item));
}

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
TEST(Compiler, start_at_zero)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;

 {
  test::Writable_Database db(file);
  const auto lille = db.new_city("Lille");
  EXPECT_EQ(lille.get_record_id(), joedb::Record_Id{0});
  EXPECT_EQ(lille.get_id(), 0);
  const auto person = db.new_person("Rémi", lille);
  EXPECT_EQ(person.get_id(), 0);
  EXPECT_EQ(db.get_home(person).get_id(), 0);
  EXPECT_FALSE(db.get_home(person).is_null());
  const auto homeless = db.new_person("Arthur", db.null_city());
  EXPECT_TRUE(db.get_home(homeless).is_null());
  db.soft_checkpoint();
 }

 {
  test::Readonly_Database db(file);
  const test::id_of_city lille{0};
  EXPECT_EQ(db.get_name(lille), "Lille");
  const test::id_of_person person{0};
  const test::id_of_person homeless{1};
  EXPECT_EQ(db.get_name(person), "Rémi");
  EXPECT_EQ(db.get_name(homeless), "Arthur");
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, id_test)
/////////////////////////////////////////////////////////////////////////////
{
 test::id_of_person null_person;
 test::id_of_person non_null_person(1234);

 EXPECT_TRUE(null_person.is_null());
 EXPECT_FALSE(null_person.is_not_null());
 EXPECT_FALSE(non_null_person.is_null());
 EXPECT_TRUE(non_null_person.is_not_null());
 EXPECT_EQ(null_person.get_id(), -1);
 EXPECT_EQ(non_null_person.get_id(), 1234);
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, file_test)
/////////////////////////////////////////////////////////////////////////////
{
 //
 // First, try to open the database
 //
 joedb::Memory_File file;
 joedb::Interpreted_File("compiler/db/test.joedbi", joedb::Open_Mode::read_existing).copy_to(file);
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

 EXPECT_EQ(db.get_person_table().get_size(), 3);
 EXPECT_EQ(db.get_city_table().get_size(), 4);

 EXPECT_EQ(db.get_city_table().first().get_id(), 0);
 EXPECT_EQ(db.get_city_table().last().get_id(), 3);
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
  EXPECT_STREQ(e.what(), "my_namespace::is_nested::test: city_by_name unique index failure: (\"Paris\") at id = 1 was already at id = 2");
 }

 //
 // Validity + get_at
 //
 EXPECT_EQ(db.get_name(db.find_city_by_name(std::string_view("Paris"))), "Paris");
 db.delete_city(db.find_city_by_name("Paris"));

 EXPECT_FALSE(db.get_city_table().is_valid_at(-1));
 EXPECT_TRUE (db.get_city_table().is_valid_at(0));
 EXPECT_TRUE (db.get_city_table().is_valid_at(1));
 EXPECT_FALSE(db.get_city_table().is_valid_at(2));
 EXPECT_TRUE (db.get_city_table().is_valid_at(3));
 EXPECT_FALSE(db.get_city_table().is_valid_at(4));
 EXPECT_FALSE(db.get_city_table().is_valid_at(5));

 EXPECT_EQ(db.get_name(test::container_of_city::get_at(0)), "Barcelona");
 EXPECT_EQ(db.get_name(test::container_of_city::get_at(3)), "Tokyo");

 //
 // Loop over not-unique index
 //
 db.new_person("Toto", db.find_city_by_name("Tokyo"));
 db.new_person("Toto", db.find_city_by_name("Barcelona"));

 {
  int count = 0;
  for (auto toto: db.find_person_by_name(std::string_view("Toto")))
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
  EXPECT_EQ(x.get_id(), 0);
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

 db.soft_checkpoint();
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, exceptions)
/////////////////////////////////////////////////////////////////////////////
{
 try
 {
  test::Memory_Database db;
  db.new_city("Paris");
  db.new_city("Lille");
  db.new_city("Paris");
  db.soft_checkpoint();
  ADD_FAILURE() << "should have thrown";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "my_namespace::is_nested::test: city_by_name unique index failure: (\"Paris\") at id = 2 was already at id = 0");
 }

 try
 {
  multi_index::Memory_Database db;
  db.new_person("Chantal", "Dupont", db.null_city());
  db.new_person("Rémi", "Coulom", db.null_city());

  for (const auto person: db.find_person_by_name("Coulom"))
   EXPECT_EQ(db.get_first_name(person), "Rémi");

  db.new_person("Rémi", "Munos", db.null_city());
  db.new_person("Marcel", "Coulom", db.null_city());
  db.new_person("Albert", "Premier", db.null_city());
  db.new_person("Rémi", "Coulom", db.null_city());
  db.soft_checkpoint();
  ADD_FAILURE() << "should have thrown";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "multi_index: person_by_full_name unique index failure: (\"Rémi\", \"Coulom\") at id = 5 was already at id = 1");
 }

 try
 {
  test::Memory_Database db;
  auto translation = db.new_translation();
  ((joedb::Writable *)&db)->delete_from(joedb::Table_Id(5), translation.get_record_id());
  db.soft_checkpoint();
 }
 catch (const joedb::Exception &e)
 {
  ADD_FAILURE() << e.what();
 }

 try
 {
  test::Memory_Database db;
  ((joedb::Writable *)&db)->insert_into(joedb::Table_Id(1), joedb::Record_Id(1));
  ((joedb::Writable *)&db)->insert_into(joedb::Table_Id(1), joedb::Record_Id(1));
  db.soft_checkpoint();
  ADD_FAILURE() << "should have thrown";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "my_namespace::is_nested::test: Duplicate insert into table city");
 }

 try
 {
  test::Memory_Database db;
  ((joedb::Writable *)&db)->insert_into(joedb::Table_Id(5), joedb::Record_Id(1));
  ((joedb::Writable *)&db)->insert_into(joedb::Table_Id(5), joedb::Record_Id(3));
  db.soft_checkpoint();
 }
 catch (const joedb::Exception &e)
 {
  ADD_FAILURE() << e.what();
 }

 try
 {
  test::Memory_Database db;
  db.set_max_record_id(1000);
  ((joedb::Writable *)&db)->insert_into(joedb::Table_Id(1), joedb::Record_Id(2000));
  db.soft_checkpoint();
  ADD_FAILURE() << "should have thrown";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "my_namespace::is_nested::test: insert_into: too big");
 }

 try
 {
  test::Memory_Database db;
  db.set_max_record_id(1000);
  ((joedb::Writable *)&db)->insert_vector(joedb::Table_Id(1), joedb::Record_Id(1), 2000);
  db.soft_checkpoint();
  ADD_FAILURE() << "should have thrown";
 }
 catch (const joedb::Exception &e)
 {
  EXPECT_STREQ(e.what(), "my_namespace::is_nested::test: insert_vector: null record_id, or too big");
 }

#ifndef NDEBUG
 try
 {
  test::Memory_Database db;
  auto city = db.new_city("Paris");
  EXPECT_STREQ(db.get_name(city).c_str(), "Paris");
  db.delete_city(city);
  db.soft_checkpoint();
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
  test::Memory_Database db;
  auto city = db.new_city("Paris");
  db.delete_city(city);
  db.soft_checkpoint();
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
  test::Memory_Database db;
  auto city = db.new_city("Paris");
  db.delete_city(city);
  db.soft_checkpoint();
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
  joedb::Interpreted_File("compiler/db/test.joedbi", joedb::Open_Mode::read_existing)
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
 test::Memory_Database db;
 EXPECT_EQ(db.ahead_of_checkpoint(), 0);
 db.hard_checkpoint();
 EXPECT_EQ(db.ahead_of_checkpoint(), 0);
 db.new_city("Paris");
 EXPECT_TRUE(db.ahead_of_checkpoint() > 0);
 db.hard_checkpoint();
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
  db.soft_checkpoint();
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
  db.soft_checkpoint();
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
  db.soft_checkpoint();
 }

 joedb::Memory_File server_file;

 {
  test::Writable_Database db(server_file);
  db.new_person("X", db.null_city());
  db.soft_checkpoint();
 }

 joedb::File_Connection connection(server_file);

 try
 {
  test::Client client(client_file, connection);
  ADD_FAILURE() << "Should have thrown\n";
 }
 catch (const joedb::Content_Mismatch &)
 {
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, vector_of_size_zero)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;

 vector_test::id_of_point v;

 {
  vector_test::Writable_Database db(file);
  v = db.new_vector_of_point(0);
  db.soft_checkpoint();
 }
 {
  vector_test::Readonly_Database db(file);
  EXPECT_EQ(db.get_point_table().get_size(), 0ULL);
 }
 {
  joedb::Database db;
  joedb::Readonly_Journal(file).replay_log(db);
 }

 {
  vector_test::Writable_Database db(file);
  db.update_vector_of_x(v, 0, [](joedb::Span<float> x)
  {
  });
  db.soft_checkpoint();
 }
 {
  vector_test::Readonly_Database db(file);
  EXPECT_EQ(db.get_point_table().get_size(), 0ULL);
 }
 {
  joedb::Database db;
  joedb::Readonly_Journal(file).replay_log(db);
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, vector_update_rejects_invalid_range_before_callback)
/////////////////////////////////////////////////////////////////////////////
{
#if defined(NDEBUG) && !defined(JOEDB_FUZZING)
 GTEST_SKIP() << "Requires debug assertions";
#else
 joedb::Memory_File file;
 vector_test::Writable_Database db(file);
 const auto v = db.new_vector_of_point(1);
 db.soft_checkpoint();

 bool callback_called = false;
 EXPECT_THROW
 (
  db.update_vector_of_x(v, 2, [&](joedb::Span<float>)
  {
   callback_called = true;
  }),
  joedb::Assertion_Failure
 );

 EXPECT_FALSE(callback_called);
 EXPECT_EQ(db.ahead_of_checkpoint(), 0);
#endif
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
   db.soft_checkpoint();
  }

  {
   test::Readonly_Database db(file);
   for (size_t i = 0; i < n; i++)
    EXPECT_FLOAT_EQ(db.get_value(v[i]), 0.1f * float(i));
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
   db.soft_checkpoint();

   EXPECT_EQ(db.get_point_table().first().get_id(), 0ULL);
   EXPECT_EQ(db.get_point_table().last().get_id(), 4ULL);
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

   db.soft_checkpoint();
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
  joedb::Interpreted_File("compiler/vector_hole.joedbi", joedb::Open_Mode::read_existing).copy_to(file);
  vector_test::Writable_Database db(file);
 }
 catch (const joedb::Exception &e)
 {
  ADD_FAILURE() << e.what();
 }

 try
 {
  joedb::Memory_File file;
  joedb::Interpreted_File("compiler/vector_hole_by_vector_insert.joedbi", joedb::Open_Mode::read_existing).copy_to(file);

  {
   vector_test::Writable_Database db(file);
   db.set_x(db.get_point_table().first(), 1.234f);
   db.soft_checkpoint();
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
  joedb::Interpreted_File("compiler/vector_delete.joedbi", joedb::Open_Mode::read_existing).copy_to(file);
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

  db.soft_checkpoint();

  {
   vector_test::Readonly_Database rdb(file);
   const auto remi = rdb.find_person_by_name("Rémi");
   if (remi.is_not_null())
    EXPECT_EQ(rdb.get_name(remi), "Rémi");
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

  db.soft_checkpoint();
 }

 //
 // timestamp
 //
 {
  vector_test::Memory_Database db;
  db.write_timestamp();
  db.write_comment("This was a timestamp.");
  db.soft_checkpoint();
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, index_iteration)
/////////////////////////////////////////////////////////////////////////////
{
 test::Memory_Database db;

 const auto tokyo = db.new_city("Tokyo");
 const auto lille = db.new_city("Lille");
 const auto abidjan = db.new_city("Abidjan");
 const auto paris = db.new_city("Paris");

 db.soft_checkpoint();

 const test::Database &cdb(db);

 EXPECT_EQ(cdb.next_city_by_name(lille), paris);
 EXPECT_EQ(cdb.next_city_by_name(paris), tokyo);
 EXPECT_EQ(cdb.previous_city_by_name(lille), abidjan);
 EXPECT_EQ(cdb.previous_city_by_name(abidjan), db.null_city());

 EXPECT_EQ(cdb.next(tokyo), lille);
 EXPECT_EQ(cdb.previous(lille), tokyo);
 EXPECT_EQ(cdb.next(paris), db.get_city_table().get_end());
}

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

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, delete_vector)
/////////////////////////////////////////////////////////////////////////////
{
 test::Memory_Database db;

 const auto v = db.new_vector_of_city(100);

 const size_t first = 20;
 const size_t size = 40;

 db.delete_vector_of_city(v[first], size);
 db.soft_checkpoint();

 for (size_t i = 0; i < 100; i++)
 {
  const auto city = v[i];
  if (city >= v[first] && city < v[first + size])
   EXPECT_FALSE(db.is_valid(city));
  else
   EXPECT_TRUE(db.is_valid(city));
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, construction_flags)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;

 {
  test::Writable_Database db(file);
  const auto person = db.new_person();
  db.set_name(person, "Joe");
  db.flush();
 }

 EXPECT_ANY_THROW(test::Writable_Database{file});

 {
  test::Writable_Database db(file, joedb::Recovery::overwrite);
  EXPECT_EQ(db.get_person_table().get_size(), 0);
  const auto person = db.new_person();
  db.set_name(person, "Bob");
  db.soft_checkpoint();
 }

 {
  test::Writable_Database db(file);
  EXPECT_EQ(db.get_person_table().get_size(), 1);
  EXPECT_EQ(db.get_name(*db.get_person_table().begin()), "Bob");
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, insert)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 const test::id_of_person person{123};

 {
  test::Writable_Database db(file);
  db.insert_person(person);
  db.set_name(person, "Joe");
  db.soft_checkpoint();
 }

 {
  test::Readonly_Database db(file);
  EXPECT_EQ("Joe", db.get_name(person));
 }
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, id_hash)
/////////////////////////////////////////////////////////////////////////////
{
 std::unordered_map<test::id_of_person, int> map;
}

/////////////////////////////////////////////////////////////////////////////
#if __cplusplus >= 202002L
TEST(Compiler, unordered_index)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 unordered_index::Writable_Database db(file);

 const auto remi = db.new_person("Rémi", "Coulom", db.null_city());
 const auto joe = db.new_person("Joe", "Le Taxi", db.null_city());

 EXPECT_EQ(remi, db.find_person_by_full_name("Rémi", "Coulom", db.null_city()));
 EXPECT_EQ(joe, db.find_person_by_full_name("Joe", "Le Taxi", db.null_city()));
 EXPECT_EQ(db.null_person(), db.find_person_by_full_name("first", "last", db.null_city()));

 for (const auto person: db.find_person_by_name("Coulom"))
  EXPECT_EQ(db.get_first_name(person), "Rémi");
}

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, unordered_index_string_view)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 unordered_index::Writable_Database db(file);

 // Include an embedded null and leave a suffix outside each lookup view.
 const std::string first_name("Jo\0e!", 5);
 const std::string last_name("Smith!");
 const std::string_view first(first_name.data(), 4);
 const std::string_view last(last_name.data(), 5);
 const auto person = db.new_person(std::string(first), std::string(last), db.null_city());

 EXPECT_EQ(person, db.find_person_by_first_name(first));
 EXPECT_EQ(person, db.find_person_by_full_name(first, last, db.null_city()));
 EXPECT_EQ(db.null_person(), db.find_person_by_first_name(std::string_view(first_name)));
 EXPECT_EQ(db.null_person(), db.find_person_by_full_name(first, std::string_view(last_name), db.null_city()));

 size_t count = 0;
 for (const auto match: db.find_person_by_name(last))
 {
  EXPECT_EQ(person, match);
  ++count;
 }
 EXPECT_EQ(count, 1);

 count = 0;
 for (const auto match: db.find_people_by_full_name(first, last, db.null_city()))
 {
  EXPECT_EQ(person, match);
  ++count;
 }
 EXPECT_EQ(count, 1);

 for (const auto match: db.find_people_by_full_name(first, std::string_view(last_name), db.null_city()))
  ADD_FAILURE() << "Unexpected match: " << match.get_record_id();
}
#endif
