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
  std::cout << city.get_name(db) << '\n';

 auto Amsterdam = db.new_city();
 Amsterdam.set_name(db, "Amsterdam");

 auto aristide = db.new_person();
 aristide.set_name(db, "Aristide");
 aristide.set_home(db, Amsterdam);

 std::cout << "\nList of persons with their cities:\n";
 for (auto person: db.get_person_table())
 {
  std::cout << person.get_name(db) << ' ';
  auto home = person.get_home(db);
  if (home.is_null())
   std::cout << "is homeless\n";
  else
   std::cout << "lives in " << home.get_name(db) << '\n';
 }

 return 0;
}
