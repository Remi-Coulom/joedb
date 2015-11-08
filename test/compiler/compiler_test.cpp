#include "testdb.h"
#include "File.h"
#include "Journal_File.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
void print_table_sizes(const testdb::Database &db)
/////////////////////////////////////////////////////////////////////////////
{
 std::cout << '\n';
 std::cout << "Dump\n";
 std::cout << "====\n";
 std::cout << " Persons: " << db.get_person_table().get_size();
 std::cout << " (is_empty = " << db.get_person_table().is_empty() << ')';
 std::cout << '\n';
 for (auto person: db.get_person_table())
  std::cout << "  " << db.get_name(person) << '\n';

 std::cout << '\n';
 std::cout << " Cities: " << db.get_city_table().get_size() << '\n';
 for (auto city: db.get_city_table())
  std::cout << "  " << db.get_name(city) << '\n';
}

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 std::cout << "\nTesting compiled code...\n";

 //
 // First, try to open the database
 //
 testdb::Database db("test.joedb");
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
 print_table_sizes(db);

 //
 // Manually remove persons
 //
 db.delete_person(*db.get_person_table().begin());
 db.delete_person(*db.get_person_table().begin());
 db.delete_person(*db.get_person_table().begin());
 print_table_sizes(db);

 //
 // Clear Persons and cities
 //
 db.clear_city_table();
 db.clear_person_table();

 //
 // Print table sizes
 //
 print_table_sizes(db);

 //
 // Insert some
 //
 db.new_city("Barcelona");
 auto New_York = db.new_city("New York");
 db.new_person("Toto", New_York);
 print_table_sizes(db);

 return 0;
}
