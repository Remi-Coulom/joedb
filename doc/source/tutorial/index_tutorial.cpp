#include "tutorial/Buffered_File_Database.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 tutorial::Buffered_File_Database db(file);

 //
 // Insert some cities
 //
 db.new_city("Paris");
 db.new_city("Tokyo");
 db.new_city("Chicago");

 try
 {
  std::cout << "Trying to insert another Paris:\n";
  db.new_city("Paris");
 }
 catch(const std::runtime_error &e)
 {
  std::cout << "Exception: " << e.what() << '\n';
 }

 //
 // Finding a city by name
 //
 const auto Paris = db.find_city_by_name("Paris");
 const auto Tokyo = db.find_city_by_name("Tokyo");
 if (db.find_city_by_name("Monte Carlo").is_null())
  std::cout << "\nMonte Carlo is not in the database\n";

 //
 // Inserting persons
 //
 db.new_person("John", "Smith", Paris);
 db.new_person("John", "Smith", Tokyo);
 db.new_person("Hiroshi", "Yamada", Tokyo);
 db.new_person("René", "Dubois", Tokyo);
 db.new_person("Hélène", "Dubois", db.null_city());
 db.new_person("Daniel", "Dubois", db.null_city());
 db.new_person("Laurent", "Dubois", db.null_city());
 db.new_person("Albert", "Camus", Paris);

 //
 // Finding persons with the index
 //
 std::cout << "\nFinding all the John Smiths:\n";
 for (const auto person: db.find_person_by_name("Smith", "John"))
 {
  std::cout << db.get_first_name(person) << ' ';
  std::cout << db.get_last_name(person) << ", ";
  std::cout << db.get_name(db.get_home(person)) << '\n';
 }

 //
 // Using the index to sort persons
 //
 std::cout << "\nSorted list of persons:\n";
 for (const auto &[name, person]: db.get_index_of_person_by_name())
 {
  const auto &[last, first] = name;
  std::cout << last << ", " << first << '\n';
 }

 db.checkpoint();

 return 0;
}
