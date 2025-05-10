#include "tutorial/File_Database.h"
#include "tutorial/Readonly_Database.h"

#include "joedb/ui/main_wrapper.h"

/////////////////////////////////////////////////////////////////////////////
static int file_tutorial_main(joedb::Arguments &arguments)
/////////////////////////////////////////////////////////////////////////////
{
 const char * const file_name = "file_tutorial.joedb";

 //
 // Create a new database and write something
 // (If a file already exists, it will fail)
 //
 {
  tutorial::File_Database db(file_name, joedb::Open_Mode::create_new);
  db.new_city("Villeneuve d'Ascq");
  db.soft_checkpoint();
 }

 //
 // Re-open the database and add one more city
 // (If the file does not exist, it will fail)
 //
 {
  tutorial::File_Database db(file_name, joedb::Open_Mode::write_existing);
  db.new_city("Tombouctou");
  db.soft_checkpoint();
 }

 //
 // Open the database read-only
 //
 {
  tutorial::Readonly_Database db(file_name);
  for (const auto city: db.get_city_table())
   std::cout << db.get_name(city) << '\n';
 }

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
/////////////////////////////////////////////////////////////////////////////
{
 return joedb::main_wrapper(file_tutorial_main, argc, argv);
}
