#include "testdb.h"
#include "schema_v1.h"
#include "schema_v2.h"
#include "vector_test.h"

#include "File.h"
#include "Journal_File.h"
#include "Multiplexer.h"
#include "Database.h"
#include "DB_Listener.h"
#include "translation.h"

#include <iostream>
#include <algorithm>

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
 record_id_t string_id_id,
 record_id_t language_id
)
{
 const testdb::string_id_t string_id(string_id_id);
 const testdb::language_t language(language_id);
 const testdb::language_t english(translation::language::en);

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
 if (!db.is_good())
 {
  std::cerr << "Error opening database\n";
  return 1;
 }
 else
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
 db.new_person("Évariste", testdb::city_t());
 db.new_person("Catherine", testdb::city_t());
 dump(db);

 //
 // Test unique index constraint
 //
 std::cout << "Trying to insert a duplicate Paris: ...\n";
 try
 {
  db.new_city("Paris");
 }
 catch(std::runtime_error e)
 {
  std::cout << e.what() << '\n';
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

 //
 // Standard find algorithm
 //
 {
  auto i = std::find_if
           (
            db.get_person_table().begin(),
            db.get_person_table().end(),
            [&](testdb::person_t person)
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
 db.new_person("Zoé", testdb::city_t());
 db.new_person("Albert", testdb::city_t());
 auto by_name = [&](testdb::person_t p_1, testdb::person_t p_2)
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

  db.delete_city(v[0]);
  db.delete_city(v[1]);
 }

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int multiplexer_test()
/////////////////////////////////////////////////////////////////////////////
{
 std::cout << "\nMultiplexer test...\n";

 joedb::File file("test.joedb", joedb::File::mode_t::write_existing);
 if (file.get_status() != joedb::File::status_t::success)
 {
  std::cout << "Error: could not open file\n";
  return 1;
 }
 testdb::Database compiled_db;
 joedb::Database interpreted_db;

 joedb::Journal_File journal(file);
 joedb::DB_Listener interpreted_listener(interpreted_db);

 joedb::Multiplexer multiplexer;
 joedb::Listener &journal_multiplexer = multiplexer.add_listener(journal);
 joedb::Listener &compiled_multiplexer = multiplexer.add_listener(compiled_db);
 joedb::Listener &interpreted_multiplexer = multiplexer.add_listener(interpreted_listener);

 compiled_db.set_listener(compiled_multiplexer);
 interpreted_db.set_listener(interpreted_multiplexer);

 journal.replay_log(journal_multiplexer);

 std::cout << "Tables:\n";
 for (auto table: interpreted_db.get_tables())
  std::cout << ' ' << table.second.get_name() << '\n';

 compiled_db.new_city("Multiplexer City");

 table_id_t city_id = interpreted_db.find_table("city");
 const joedb::Table &city_table = interpreted_db.get_tables().find(city_id)->second;
 field_id_t name_id = city_table.find_field("name");
 const joedb::Field &name_field = city_table.get_fields().find(name_id)->second;

 for (size_t i = 1; i <= compiled_db.get_city_table().get_size(); i++)
  std::cout << i << ' ' << name_field.get_string(record_id_t(i)) << '\n';

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
void schema_v2::File_Database::set_default_preferred_language_to_english
/////////////////////////////////////////////////////////////////////////////
(
 Database &db
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
  if (!db.is_good())
  {
   std::cout << "Error during opening of v1\n";
   return 1;
  }
  std::cout << "v1 opened\n";

  db.new_person("Toto");
  db.new_person("Rémi");
 }

 {
  schema_v1::File_Database db("upgrade_test.joedb");
  if (!db.is_good())
  {
   std::cout << "Error during re-opening of v1\n";
   return 1;
  }
  std::cout << "v1 re-opened\n";
 }

 {
  schema_v2::File_Database db("upgrade_test.joedb");
  if (!db.is_good())
  {
   std::cout << "Error during opening of v2\n";
   return 1;
  }
  else
   std::cout << "v2 opened\n";
 }

 {
  schema_v2::File_Database db("upgrade_test.joedb");
  if (!db.is_good())
  {
   std::cout << "Error during opening of v2\n";
   return 1;
  }
  else
   std::cout << "v2 re-opened\n";
 }

 {
  schema_v1::File_Database db("upgrade_test.joedb");
  if (db.is_good())
  {
   std::cout << "Error: v1 code should not open v2 files\n";
   return 1;
  }
  else
   std::cout << "OK: v1 code does not open v2 file\n";
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
  testdb::float_t v;

  {
   testdb::File_Database db("test.joedb");
   v = db.new_vector_of_float(n);
   for (size_t i = 0; i < n; i++)
    db.set_value(v[i], 0.1f * float(i));
  }

  {
   testdb::File_Database db("test.joedb", true);
   for (size_t i = 0; i < n; i++)
    std::cout << "v[" << i << "] = " << db.get_value(v[i]) << '\n';
  }
 }

 //
 // Vector storage
 //
 {
  vector_test::point_t v;

  {
   vector_test::File_Database db("vector_test.joedb");
   v = db.new_vector_of_point(n);
   for (size_t i = 0; i < n; i++)
   {
    db.set_x(v[i], 0.1f * float(i));
    db.set_y(v[i], 1.234f);
   }
  }

  {
   vector_test::File_Database db("vector_test.joedb", true);
   for (size_t i = 0; i < n; i++)
   {
    std::cout << "v[" << i << "] = {" << db.get_x(v[i]);
    std::cout << ", " << db.get_y(v[i]) << "}\n";
   }
  }
 }

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 return file_test() || multiplexer_test() || schema_upgrade_test() || do_vector_test();
}
