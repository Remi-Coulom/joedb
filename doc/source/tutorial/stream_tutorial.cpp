#include "tutorial.h"

#include <fstream>

/////////////////////////////////////////////////////////////////////////////
int main()
/////////////////////////////////////////////////////////////////////////////
{
 const char * const file_name = "stream_tutorial.joedb";

 //
 // Create the database and write something
 // (If a file already exists, it will be erased)
 //
 {
  std::fstream stream
  (
   file_name,
   std::ios::binary | std::ios::out
  );

  joedb::Stream_File file(stream, joedb::Open_Mode::create_new);
  tutorial::Generic_File_Database db(file);
  db.new_city("Villeneuve d'Ascq");
 }

 //
 // Re-open the database and add one more city
 // (If the file does not exist, it will fail)
 //
 {
  std::fstream stream
  (
   file_name,
   std::ios::binary | std::ios::out | std::ios::in
  );

  joedb::Stream_File file(stream, joedb::Open_Mode::write_existing);
  tutorial::Generic_File_Database db(file);
  db.new_city("Tombouctou");
 }

 //
 // Use a C++ istream to re-open the database read-only
 //
 {
  std::ifstream stream
  (
   file_name,
   std::ios::binary | std::ios::in
  );

  joedb::Input_Stream_File file(stream);
  tutorial::Readonly_Database db(file);
  for (auto city: db.get_city_table())
   std::cout << db.get_name(city) << '\n';
 }

 return 0;
}
