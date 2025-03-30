#include "db/blob/Writable_Database.h"
#include "db/blob/Readonly_Database.h"
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
  blob::Writable_Database db(file);
  const auto person = db.new_person();
  const joedb::Blob name_blob = db.write_blob_data("Jacques");
  db.set_name(person, name_blob);
  EXPECT_EQ("Jacques", db.read_blob_data(name_blob));
  db.set_city(person, "Paris");
  db.checkpoint();
 }

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

 //
 // It is possible to read directly from the db when it is not read-only
 //
 {
  blob::Writable_Database db(file);

  const auto person = db.get_person_table().first();
  const joedb::Blob name_blob = db.get_name(person);
  EXPECT_EQ("Jacques", db.read_blob_data(name_blob));
 }
}
