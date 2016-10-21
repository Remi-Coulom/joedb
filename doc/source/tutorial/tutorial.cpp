#include "tutorial.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
int tutorial_function()
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
 // Loop over rows of a table
 //
 std::cout << "List of cities:\n";
 for (auto city: db.get_city_table())
  std::cout << db.get_name(city) << '\n';

 //
 // Inserting new records
 //
 auto Amsterdam = db.new_city("Amsterdam");
 auto Aristide = db.new_person("Aristide", "Martines", Amsterdam);

 //
 // Updating a value
 //
 db.set_last_name(Aristide, "Martinez");

 //
 // A join between the two tables
 //
 std::cout << "\nList of persons with their cities:\n";
 for (auto person: db.get_person_table())
 {
  std::cout << db.get_first_name(person) << ' ';
  std::cout << db.get_last_name(person) << ' ';
  auto home = db.get_home(person);
  if (home.is_null())
   std::cout << "is homeless\n";
  else
   std::cout << "lives in " << db.get_name(home) << '\n';
 }

 //
 // Deleting records
 //
 db.delete_person(Aristide);
 db.delete_city(Amsterdam);

 //
 // Finding records
 //
 std::cout << "\nLooking for cities...\n";
 static const char * const cities[] = {"Paris", "London", "New York", "Tokyo"};
 for (auto city_name: cities)
 {
  std::cout << "  " << city_name;
  auto city = db.find_city_by_name(city_name);
  if (city.is_null())
   std::cout << ": not found";
  else
   std::cout << ": id = " << city.get_id();
  std::cout << '\n';
 }

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 try
 {
  return tutorial_function();
 }
 catch (const std::exception &e)
 {
  std::cerr << "Exception caught: ";
  std::cerr << e.what() << '\n';
  return 1;
 }
}
