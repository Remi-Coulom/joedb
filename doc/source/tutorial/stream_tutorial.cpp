#include "tutorial.h"
#include "joedb/journal/Stream_File.h"

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
  std::filebuf filebuf;
  filebuf.open
  (
   file_name,
   std::ios::binary | std::ios::out
  );

  joedb::Stream_File file(filebuf, joedb::Open_Mode::create_new);
  tutorial::Generic_File_Database db(file);
  db.new_city("Villeneuve d'Ascq");
 }

 //
 // Re-open the database and add one more city
 // (If the file does not exist, it will fail)
 //
 {
  std::filebuf filebuf;
  filebuf.open
  (
   file_name,
   std::ios::binary | std::ios::out | std::ios::in
  );

  joedb::Stream_File file(filebuf, joedb::Open_Mode::write_existing);
  tutorial::Generic_File_Database db(file);
  db.new_city("Tombouctou");
 }

 //
 // Re-open the database read-only
 //
 {
  std::filebuf filebuf;
  filebuf.open
  (
   file_name,
   std::ios::binary | std::ios::in
  );

  joedb::Stream_File file(filebuf, joedb::Open_Mode::read_existing);
  tutorial::Readonly_Database db(file);
  for (auto city: db.get_city_table())
   std::cout << db.get_name(city) << '\n';
 }

 return 0;
}
