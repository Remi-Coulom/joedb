#include "testdb.h"
#include "File.h"
#include "Journal_File.h"
#include "Multiplexer.h"
#include "Database.h"
#include "DB_Listener.h"

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
 // Generic find
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

 for (int i = 1; i < 5; i++)
  std::cout << i << ' ' << name_field.get_string(record_id_t(i)) << '\n';

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 return file_test() || multiplexer_test();
}
