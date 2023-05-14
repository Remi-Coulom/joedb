#include "db/blob.h"
#include "joedb/journal/Memory_File.h"

#include "gtest/gtest.h"

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, blob)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;

 //
 // A blob is created with the write_blob_data function
 //
 {
  blob::Generic_File_Database db(file);
  const auto person = db.new_person();
  const joedb::Blob name_blob = db.write_blob_data("Jacques");
  db.set_name(person, name_blob);
  EXPECT_EQ("Jacques", db.read_blob_data(name_blob));
  db.set_city(person, "Paris");
  db.checkpoint();
 }

 file.set_mode(joedb::Open_Mode::read_existing);

 //
 // The content of a blob is not kept in RAM. In order to access it,
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
