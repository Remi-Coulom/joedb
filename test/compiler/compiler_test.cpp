#include "testdb.h"
#include "schema_v1.h"
#include "schema_v2.h"
#include "vector_test.h"
#include "multi_index.h"
#include "translation.h"

#include "joedb/journal/File.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/interpreter/Database.h"
#include "joedb/io/Interpreter_Dump_Writable.h"

#include <iostream>
#include <algorithm>

using namespace my_namespace::is_nested;

/////////////////////////////////////////////////////////////////////////////
void dump(const testdb::Database &db)
/////////////////////////////////////////////////////////////////////////////
{
 std::cout << '\n';
 std::cout << "Dump\n";
 std::cout << "====\n";
 std::cout << " Persons: " << db.get_person_table().get_size();
 std::cout << " (is_empty = " << db.get_person_table().is_empty() << ')';
 std::cout << '\n';
 for (auto person: db.get_person_table())
 {
  std::cout << "  " << person.get_id() << ": ";
  std::cout << db.get_name(person) << ' ';
  std::cout << db.get_home(person).get_id() << '\n';
 }

 std::cout << '\n';
 std::cout << " Cities: " << db.get_city_table().get_size() << '\n';
 for (auto city: db.get_city_table())
 {
  std::cout << "  " << city.get_id() << ": ";
  std::cout << db.get_name(city) << '\n';
 }

 std::cout << '\n';
}

/////////////////////////////////////////////////////////////////////////////
const std::string &get_translation
/////////////////////////////////////////////////////////////////////////////
(
 const testdb::Database &db,
 Record_Id string_id_id,
 Record_Id language_id
)
{
 const testdb::id_of_string_id string_id(string_id_id);
 const testdb::id_of_language language(language_id);
 const testdb::id_of_language english(translation::language::en);

 auto translation = db.find_translation_by_ids(string_id, language);

 if (translation.is_null())
  translation = db.find_translation_by_ids(string_id, english);

 if (!translation.is_null())
  return db.get_translation(translation);

 const static std::string error("Translation error!");
 return error;
}

/////////////////////////////////////////////////////////////////////////////
int file_test()
/////////////////////////////////////////////////////////////////////////////
{
 std::cout << "\nTesting compiled code...\n";

 //
 // First, try to open the database
 //
 testdb::File_Database db("test.joedb");
 std::cout << "Database opened successfully\n";

 //
 // Print table sizes
 //
 dump(db);

 //
 // Clear Persons and cities
 //
 db.clear_city_table();
 db.clear_person_table();

 //
 // Print table sizes
 //
 dump(db);

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
 dump(db);

 std::cout << "first: " << db.get_city_table().first().get_id() << ": ";
 std::cout << db.get_name(db.get_city_table().first()) << '\n';
 std::cout << "last: " << db.get_city_table().last().get_id() << ": ";
 std::cout << db.get_name(db.get_city_table().last()) << '\n';

 //
 // Test unique index constraint
 //
 std::cout << "Trying to update into a duplicate Paris: ...\n";
 try
 {
  db.set_name(New_York, "Paris");
 }
 catch(const std::runtime_error &e)
 {
  std::cout << e.what() << '\n';
  db.set_name(New_York, "New York");
 }
 dump(db);

 //
 // Validity + get_at
 //
 std::cout << "\nValidity + get_at:\n";
 std::cout << db.get_name(db.find_city_by_name("Paris")) << '\n';
 db.delete_city(db.find_city_by_name("Paris"));
 for (size_t i = 0; i < 10; i++)
 {
  bool valid = db.get_city_table().is_valid_at(i);
  std::cout << i << ": ";
  if (valid)
   std::cout << db.get_name(db.get_city_table().get_at(i));
  else
   std::cout << "invalid!";
  std::cout << '\n';
 }

 //
 // Loop over not-unique index
 //
 db.new_person("Toto", db.find_city_by_name("Tokyo"));
 db.new_person("Toto", db.find_city_by_name("Barcelona"));
 dump(db);

 std::cout << "Finding all Totos:\n";
 for (auto toto: db.find_person_by_name("Toto"))
 {
  std::cout << ' ' << toto.get_id();
  auto city = db.get_home(toto);
  if (!city.is_null())
   std::cout << ": " << db.get_name(city);
  std::cout << '\n';
 }

 if (db.find_person_by_name("Totox").empty())
  std::cout << "No Totox in the database\n";

 {
  const auto range = db.find_person_by_name("Toto");
  int count = 0;
  for (auto x = range.begin(); x != range.end(); ++x)
   count++;
  std::cout << "(begin != end) = " << (range.begin() != range.end()) << '\n';
  std::cout << "empty = " << range.empty() << '\n';
  std::cout << "count = " << count << '\n';
  std::cout << "range.size() = " << range.size() << '\n';
 }

 //
 // Standard find algorithm
 //
 {
  auto i = std::find_if
           (
            db.get_person_table().begin(),
            db.get_person_table().end(),
            [&](testdb::id_of_person person)
            {
             return db.get_name(person) == "Catherine";
            }
           );
  if (i != db.get_person_table().end())
  {
   std::cout << "Found Catherine!\n";
  }
 }

 //
 // Isn't it simpler with a plain loop?
 //
 for (auto person: db.get_person_table())
  if (db.get_name(person) == "Catherine")
  {
   std::cout << "Found Catherine!\n";
   break;
  }

 //
 // Translation test
 //
 std::cout << get_translation
              (
               db,
               translation::how_are_you,
               translation::language::fr
              );
 std::cout << '\n';

 //
 // Sorting
 //
 std::cout << "Sorting with explicit sort:\n";
 db.new_person("Zoé", db.null_city());
 db.new_person("Albert", db.null_city());
 auto by_name = [&](testdb::id_of_person p_1, testdb::id_of_person p_2)
                {
                 return db.get_name(p_1) < db.get_name(p_2);
                };
 for (auto person: db.sorted_person(by_name))
  std::cout << db.get_name(person) << '\n';

 //
 // Sorting with index
 //
 std::cout << "Sorting with index:\n";
 for (auto &x: db.get_index_of_person_by_name())
  std::cout << db.get_name(x.second) << '\n';

 //
 // Inserting a vector with a unique index
 //
 {
  auto v = db.new_vector_of_city(2);
  db.set_name(v[0], "Washington");
  db.set_name(v[1], "Beijing");

  std::cout << "Reverse order of cities:\n";
  for (auto x = db.get_index_of_city_by_name().rbegin();
       x != db.get_index_of_city_by_name().rend();
       ++x)
   std::cout << db.get_name(x->second) << '\n';
 }

 //
 // New element in table with no field
 //
 {
  auto x = db.new_table_with_no_field();
  std::cout << "id in table_with_no_field: " << x.get_id() << '\n';
 }

 //
 // Comparison operators
 //
 {
  auto W = db.find_city_by_name("Washington");
  auto B = db.find_city_by_name("Beijing");
  std::cout << "W == B: " << (W == B) << '\n';
  std::cout << "W != B: " << (W != B) << '\n';
  std::cout << "W < B: " << (W < B) << '\n';
  std::cout << "W <= B: " << (W <= B) << '\n';
  std::cout << "W > B: " << (W > B) << '\n';
  std::cout << "W >= B: " << (W >= B) << '\n';
  std::cout << "B == B: " << (B == B) << '\n';
  std::cout << "B != B: " << (B != B) << '\n';
  std::cout << "B < B: " << (B < B) << '\n';
  std::cout << "B <= B: " << (B <= B) << '\n';
  std::cout << "B > B: " << (B > B) << '\n';
  std::cout << "B >= B: " << (B >= B) << '\n';
 }

 return 0;
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
int schema_upgrade_test()
/////////////////////////////////////////////////////////////////////////////
{
 {
  schema_v1::File_Database db("upgrade_test.joedb");
  std::cout << "v1 opened\n";

  db.new_person("Toto");
  db.new_person("Rémi");
  db.write_comment("This is a comment");
  db.write_timestamp(12345);
 }

 try
 {
  schema_v2::Readonly_Database db("upgrade_test.joedb");
  std::cout << "Error: v2 should not open v1 readonly without upgrade\n";
 }
 catch (const joedb::Exception &e)
 {
  std::cout << e.what() << '\n';
  std::cout << "OK: v2 refuses to open v1 readonly.\n";
 }

 {
  schema_v1::File_Database db("upgrade_test.joedb");
  std::cout << "v1 re-opened\n";
 }

 {
  schema_v2::File_Database db("upgrade_test.joedb");
  std::cout << "v2 opened\n";
 }

 {
  schema_v2::File_Database db("upgrade_test.joedb");
  std::cout << "v2 re-opened\n";
 }

 try
 {
  schema_v1::File_Database db("upgrade_test.joedb");
  std::cout << "Error: v1 code should not open v2 files\n";
 }
 catch (const joedb::Exception &e)
 {
  std::cout << e.what() << '\n';
  std::cout << "OK: v1 code does not open v2 file\n";
 }

 try
 {
  schema_v1::Readonly_Database db("upgrade_test.joedb");
  std::cout << "Error: v1 code should not open v2 files\n";
 }
 catch (const joedb::Exception &e)
 {
  std::cout << e.what() << '\n';
  std::cout << "OK: v1 code does not open v2 file\n";
 }


 {
  joedb::File file("upgrade_test.joedb", joedb::Open_Mode::read_existing);
  joedb::Readonly_Journal journal(file);
  joedb::Interpreter_Dump_Writable writable(std::cout);
  journal.replay_log(writable);
 }

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int do_vector_test()
/////////////////////////////////////////////////////////////////////////////
{
 const size_t n = 5;

 //
 // Freedom_keeper version
 //
 {
  testdb::id_of_float v;

  {
   testdb::File_Database db("test.joedb");
   v = db.new_vector_of_float(n);
   for (size_t i = 0; i < n; i++)
    db.set_value(v[i], 0.1f * float(i));
  }

  {
   testdb::Readonly_Database db("test.joedb");
   for (size_t i = 0; i < n; i++)
    std::cout << "v[" << i << "] = " << db.get_value(v[i]) << '\n';
  }
 }

 //
 // Allocating empty vectors works
 //
 {
  {
   vector_test::File_Database db("empty_vector.joedb");
   db.new_vector_of_point(0);
  }
  {
   vector_test::File_Database db("empty_vector.joedb");
   std::cout << "empty size = " << db.get_point_table().get_size() << '\n';
  }
 }

 //
 // Vector storage
 //
 {
  {
   vector_test::File_Database db("vector_test.joedb");
   auto v = db.new_vector_of_point(n);
   for (size_t i = 0; i < n; i++)
   {
    db.set_x(v[i], 0.1f * float(i));
    db.set_y(v[i], 1.234f);
   }
   std::cout << "first().get_id() = ";
   std::cout << db.get_point_table().first().get_id() << '\n';
   std::cout << "last().get_id() = ";
   std::cout << db.get_point_table().last().get_id() << '\n';
  }

  {
   vector_test::File_Database db("vector_test.joedb");
   auto v = db.new_vector_of_point(n);
   auto x = db.update_vector_of_x(v, n);
   auto y = db.update_vector_of_y(v, n);

   for (size_t i = 0; i < n; i++)
   {
    x[i] = 0.2f * float(i);
    y[i] = 5.678f;
   }
  }

  {
   vector_test::Readonly_Database db("vector_test.joedb");
   auto v = db.get_point_table().first();
   for (size_t i = 0; i < 2 * n; i++)
   {
    std::cout << "v[" << i << "] = {" << db.get_x(v[i]);
    std::cout << ", " << db.get_y(v[i]) << "}\n";
   }
  }

  try
  {
   vector_test::File_Database db("vector_hole.joedb");
  }
  catch (const joedb::Exception &e)
  {
   std::cout << "Error opening vector_hole.joedb\n";
   std::cout << e.what() << '\n';
  }

  try
  {
   {
    vector_test::File_Database db("vector_hole_by_vector_insert.joedb");
    db.set_x(db.get_point_table().first(), 1.234f);
   }
   {
    joedb::File file("vector_hole_by_vector_insert.joedb", joedb::Open_Mode::read_existing);
    joedb::Readonly_Journal journal(file);
    joedb::Database database;
    journal.replay_log(database);
   }
  }
  catch (const joedb::Exception &e)
  {
   std::cout << "Error opening vector_hole_by_vector_insert.joedb\n";
   std::cout << e.what() << '\n';
  }

  try
  {
   vector_test::File_Database db("vector_delete.joedb");
  }
  catch (const joedb::Exception &e)
  {
   std::cout << "Error opening vector_delete.joedb\n";
   std::cout << e.what() << '\n';
  }
 }

 //
 // Vector of strings with a unique index
 //
 {
  joedb::Memory_File file;
  vector_test::Generic_File_Database db(file);

  {
   constexpr int n = 3;
   auto v = db.new_vector_of_person(n);
   auto name = db.update_vector_of_name(v, n);
   name[0] = "Rémi";
   name[1] = "Paul";
   name[2] = "Liza";
  }

  {
   auto remi = db.find_person_by_name("Rémi");
   if (remi)
    std::cout << db.get_name(remi) << '\n';
   else
    std::cout << "Rémi not found\n";
  }

  {
   constexpr int n = 3;
   auto v = db.new_vector_of_person(n);
   auto name = db.update_vector_of_name(v, n);
   name[0] = "Joe";
   name[1] = "Max";
   name[2] = "Liz";
  }

  {
   auto remi = db.find_person_by_name("Rémi");
   if (remi)
    std::cout << db.get_name(remi) << '\n';
   else
    std::cout << "Rémi not found\n";
  }
 }

 //
 // timestamp
 //
 {
  vector_test::File_Database db("vector_test.joedb");
  db.write_timestamp();
  db.write_comment("This was a timestamp.");
 }

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int exceptions()
/////////////////////////////////////////////////////////////////////////////
{
 std::cout << "Testing exceptions...\n";

 try
 {
  testdb::File_Database db("unique_index_failure.joedb");
  db.new_city("Paris");
  db.new_city("Lille");
  db.new_city("Paris");
 }
 catch (const joedb::Exception &e)
 {
  std::cout << e.what() << '\n';
 }

 try
 {
  multi_index::File_Database db("multi_index_failure.joedb");
  db.new_person("Chantal", "Dupont");
  db.new_person("Rémi", "Coulom");
  db.new_person("Rémi", "Munos");
  db.new_person("Marcel", "Coulom");
  db.new_person("Albert", "Premier");
  db.new_person("Rémi", "Coulom");
 }
 catch (const joedb::Exception &e)
 {
  std::cout << e.what() << '\n';
 }

 try
 {
  testdb::File_Database db("delete_in_vector_storage.joedb");
  auto translation = db.new_translation();
  ((joedb::Writable *)&db)->delete_from(5, translation.get_id());
 }
 catch (const joedb::Exception &e)
 {
  std::cout << e.what() << '\n';
 }

 try
 {
  testdb::File_Database db("duplicate_insert.joedb");
  ((joedb::Writable *)&db)->insert_into(1, 1);
  ((joedb::Writable *)&db)->insert_into(1, 1);
 }
 catch (const joedb::Exception &e)
 {
  std::cout << e.what() << '\n';
 }

 try
 {
  testdb::File_Database db("contiguous_vector.joedb");
  ((joedb::Writable *)&db)->insert_into(5, 1);
  ((joedb::Writable *)&db)->insert_into(5, 3);
 }
 catch (const joedb::Exception &e)
 {
  std::cout << e.what() << '\n';
 }

 try
 {
  testdb::File_Database db("too_big.joedb");
  db.set_max_record_id(1000);
  ((joedb::Writable *)&db)->insert_into(1, 2000);
 }
 catch (const joedb::Exception &e)
 {
  std::cout << e.what() << '\n';
 }

 try
 {
  testdb::File_Database db("too_big.joedb");
  db.set_max_record_id(1000);
  ((joedb::Writable *)&db)->insert_vector(1, 1, 2000);
 }
 catch (const joedb::Exception &e)
 {
  std::cout << e.what() << '\n';
 }

 try
 {
  testdb::File_Database db("read_error.joedb");
  auto city = db.new_city("Paris");
  std::cout << db.get_name(city) << '\n';
  db.delete_city(city);
  std::cout << db.get_name(city) << '\n';
 }
 catch (const joedb::Assertion_Failure &)
 {
  std::cout << "Failed reading a deleted row\n";
 }

 try
 {
  testdb::File_Database db("double_delete.joedb");
  auto city = db.new_city("Paris");
  db.delete_city(city);
  db.delete_city(city);
 }
 catch (const joedb::Assertion_Failure &)
 {
  std::cout << "Double delete\n";
 }

 try
 {
  testdb::File_Database db("invalid_update.joedb");
  auto city = db.new_city("Paris");
  db.delete_city(city);
  db.set_name(city, "Paris");
 }
 catch (const joedb::Assertion_Failure &)
 {
  std::cout << "Invalid update\n";
 }

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int checkpoints()
/////////////////////////////////////////////////////////////////////////////
{
 std::cout << "Checkpoints...\n";
 testdb::File_Database db("checkpoint_test.joedb");
 std::cout << db.ahead_of_checkpoint() << '\n';
 db.checkpoint_full_commit();
 std::cout << db.ahead_of_checkpoint() << '\n';
 db.new_city("Paris");
 std::cout << db.ahead_of_checkpoint() << '\n';
 db.checkpoint_full_commit();
 std::cout << db.ahead_of_checkpoint() << '\n';

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int iterators()
/////////////////////////////////////////////////////////////////////////////
{
 testdb::File_Database db("test.joedb");

 {
  auto i = db.get_person_table().begin();
  std::cout << db.get_name(*i++) << '\n';
  std::cout << db.get_name(*i--) << '\n';
  std::cout << db.get_name(*i) << '\n';
  std::cout << db.get_name(*++i) << '\n';
  std::cout << db.get_name(*--i) << '\n';
 }

 {
  auto i = db.get_translation_table().begin();
  std::cout << db.get_translation(*i++) << '\n';
  std::cout << db.get_translation(*i--) << '\n';
  std::cout << db.get_translation(*i) << '\n';
  std::cout << db.get_translation(*++i) << '\n';
  std::cout << db.get_translation(*--i) << '\n';
 }

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 return file_test() ||
        schema_upgrade_test() ||
        do_vector_test() ||
        exceptions() ||
        checkpoints() ||
        iterators();
}
