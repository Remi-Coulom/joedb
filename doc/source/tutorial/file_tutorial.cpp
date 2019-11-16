#include "tutorial.h"

/////////////////////////////////////////////////////////////////////////////
int file_tutorial_main()
/////////////////////////////////////////////////////////////////////////////
{
 const char *file_name = "file_tutorial.joedb";

 //
 // Create a new database and write something
 // (If a file already exists, it will fail)
 //
 {
  joedb::File file(file_name, joedb::Open_Mode::create_new);
  tutorial::Generic_File_Database db(file);
  db.new_city("Villeneuve d'Ascq");
 }

 //
 // Re-open the database and add one more city
 // (If the file does not exist, it will fail)
 //
 {
  joedb::File file(file_name, joedb::Open_Mode::write_existing);
  tutorial::Generic_File_Database db(file);
  db.new_city("Tombouctou");
 }

 //
 // Open the database read-only
 //
 {
  tutorial::Readonly_Database db(file_name);
  for (auto city: db.get_city_table())
   std::cout << db.get_name(city) << '\n';
 }

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 try
 {
  return file_tutorial_main();
 }
 catch (const joedb::Exception &e)
 {
  std::cerr << "Error: " << e.what() << '\n';
  return 1;
 }
}
