#include "db/blob.h"
#include "joedb/journal/Memory_File.h"

#include "gtest/gtest.h"

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, blob)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;

 //
 // Blobs can be written into the database itself, but could also be
 // stored into another file or another joedb database.
 //
 {
  blob::Generic_File_Database db(file);
  const auto person = db.new_person();
  db.set_city(person, "Paris");
  const joedb::Blob name_blob = db.write_blob_data("Jacques");
  db.set_name(person, name_blob);
  db.checkpoint();
 }

 file.set_mode(joedb::Open_Mode::read_existing);

 //
 // The content of a blob is not stored in RAM. In order to access it,
 // it must be explicitly read from the file.
 //
 {
  blob::Readonly_Database db(file);

  const auto person = db.get_person_table().first();
  EXPECT_EQ("Paris", db.get_city(person));
  const joedb::Blob name_blob = db.get_name(person);
  EXPECT_EQ("Jacques", file.read_blob_data(name_blob));
 }
}
