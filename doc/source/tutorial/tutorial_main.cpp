#include "tutorial.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 //
 // Open the database, and test that everything is OK
 //
 tutorial::File_Database db("tutorial.joedb");
 if (!db.is_good())
 {
  std::cerr << "Error: could not open database\n";
  return 1;
 }

 //
 // Simple data manipulation
 //
 db.new_city("Tokyo");
 db.new_city("New York");
 db.new_city("Paris");

 auto Lille = db.new_city("Lille");
 auto Amsterdam = db.new_city("Amsterdam");

 db.new_person("Rémi", "Coulom", Lille);
 db.new_person("Bertrand", "Picard", Lille);

 auto Aristide = db.new_person("Aristide", "Martines", Amsterdam);

 db.set_last_name(Aristide, "Martinez");

 //
 // Use the index to display cities in alphabetical order
 //
 std::cout << "List of cities in alphabetical order:\n";
 for (auto city: db.get_index_of_city_by_name())
  std::cout << "  " << db.get_name(city.second) << '\n';

 //
 // Referring to another table
 //
 std::cout << "\nList of persons with their cities:\n";
 for (auto person: db.get_person_table())
 {
  std::cout << "  " << db.get_first_name(person) << ' ';
  std::cout << db.get_last_name(person) << ' ';
  std::cout << "lives in " << db.get_name(db.get_home(person)) << '\n';
 }

 //
 // Time stamp and comment
 //
 db.timestamp();
 db.comment("The End");

 return 0;
}
