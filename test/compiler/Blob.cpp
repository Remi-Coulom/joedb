#include "db/blob.h"
#include "joedb/journal/Memory_File.h"

#include "gtest/gtest.h"

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, blob)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;

 //
 // Blobs are set like strings
 //
 {
  blob::Generic_File_Database db(file);
  const auto person = db.new_person();
  db.set_city(person, "Paris");
  db.set_name(person, "Jacques");
  db.checkpoint();
 }

 file.set_mode(joedb::Open_Mode::read_existing);

 //
 // The content of a blob is not stored in RAM. In order to access it,
 // it must be explicitly read from the file when it is needed.
 //
 {
  blob::Readonly_Database db(file);

  const auto person = db.get_person_table().first();
  EXPECT_EQ("Paris", db.get_city(person));
  const joedb::Blob name_blob = db.get_name(person);
  EXPECT_EQ("Jacques", file.read_blob(name_blob));
 }
}
