#include "tutorial.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 //
 // Open the database
 //
 tutorial::File_Database db("tutorial.joedb");

 //
 // Simple data manipulation
 //
 db.new_city("Tokyo");
 db.new_city("New York");
 db.new_city("Paris");

 auto Lille = db.new_city("Lille");
 auto Amsterdam = db.new_city("Amsterdam");

 db.new_person("Rémi", "Coulom", Lille);
 db.new_person("Bertrand", "Picard", db.null_city());

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
  auto city = db.get_home(person);
  if (city.is_null())
   std::cout << "is homeless\n";
  else
   std::cout << "lives in " << db.get_name(city) << '\n';
 }

 //
 // Deleting a record
 //
 db.delete_city(db.find_city_by_name("New York"));

 //
 // Time stamp and comment
 //
 db.timestamp();
 db.comment("The End");

 return 0;
}
