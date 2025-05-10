#include "tutorial/File_Database.h"
#include "joedb/ui/main_exception_catcher.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
static int tutorial_main(joedb::Arguments &arguments)
/////////////////////////////////////////////////////////////////////////////
{
 //
 // Open the database
 //
 tutorial::File_Database db("tutorial.joedb", joedb::Open_Mode::create_new);

 //
 // Simple data manipulation
 //
 db.new_city("Tokyo");
 db.new_city("New York");
 db.new_city("Paris");

 const auto Lille = db.new_city("Lille");
 const auto Amsterdam = db.new_city("Amsterdam");

 db.new_person("Rémi", "Coulom", Lille);
 db.new_person("Bertrand", "Picard", db.null_city());

 const auto Aristide = db.new_person("Aristide", "Martines", Amsterdam);

 db.set_last_name(Aristide, "Martinez");

 //
 // Use the index to display cities in alphabetical order
 //
 std::cout << "List of cities in alphabetical order:\n";
 for (const auto &[name, city]: db.get_index_of_city_by_name())
  std::cout << "  " << name << '\n';

 //
 // Referring to another table
 //
 std::cout << "\nList of persons with their cities:\n";
 for (const auto person: db.get_person_table())
 {
  std::cout << "  " << db.get_first_name(person) << ' ';
  std::cout << db.get_last_name(person) << ' ';
  const auto city = db.get_home(person);
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
 db.write_timestamp();
 db.write_comment("The End");

 //
 // Writes to the database must be confirmed by an explicit checkpoint
 //
 db.soft_checkpoint();

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::main_exception_catcher(tutorial_main, argc, argv);
}
