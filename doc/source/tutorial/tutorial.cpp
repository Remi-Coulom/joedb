#include "tutorial.h"

#include <iostream>

int main()
{
 tutorial::Database db("tutorial.joedb");

 if (!db.is_good())
 {
  std::cerr << "Error: could not open database\n";
  return 1;
 }

 std::cout << "List of cities:\n";
 for (auto city: db.get_city_table())
  std::cout << db.get_name(city) << '\n';

 //
 // Inserting new records
 //
 auto Amsterdam = db.new_city();
 db.set_name(Amsterdam, "Amsterdam");
 auto aristide = db.new_person();
 db.set_name(aristide, "Aristide");
 db.set_home(aristide, Amsterdam);

 //
 // A simple join between the two tables
 //
 std::cout << "\nList of persons with their cities:\n";
 for (auto person: db.get_person_table())
 {
  std::cout << db.get_name(person) << ' ';
  auto home = db.get_home(person);
  if (home.is_null())
   std::cout << "is homeless\n";
  else
   std::cout << "lives in " << db.get_name(home) << '\n';
 }

 //
 // Deleting records
 //
 db.delete_record(aristide);
 db.delete_record(Amsterdam);

 return 0;
}
