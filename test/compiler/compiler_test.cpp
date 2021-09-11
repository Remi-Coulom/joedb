#include "db/testdb.h"
#include "translation.h"

#include "joedb/journal/File.h"
#include "joedb/journal/Generic_File.h"
#include "joedb/journal/Interpreted_File.h"
#include "joedb/journal/Memory_File.h"
#include "joedb/interpreter/Database.h"
#include "joedb/io/Interpreter_Dump_Writable.h"
#include "joedb/concurrency/Embedded_Connection.h"

#include <iostream>
#include <algorithm>

using namespace my_namespace::is_nested;

/////////////////////////////////////////////////////////////////////////////
static void dump(const testdb::Database &db)
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
static const std::string &get_translation
/////////////////////////////////////////////////////////////////////////////
(
 const testdb::Database &db,
 joedb::Record_Id string_id_id,
 joedb::Record_Id language_id
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
static int file_test()
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
   std::cout << db.get_name(testdb::container_of_city::get_at(i));
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
 for (const auto &x: db.get_index_of_person_by_name())
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

 db.checkpoint();

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 return file_test();
}
