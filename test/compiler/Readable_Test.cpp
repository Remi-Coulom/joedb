#include "db/test/Readable.h"
#include "db/test/Writable_Database.h"

using namespace my_namespace::is_nested;

#include "gtest/gtest.h"

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, Readable)
/////////////////////////////////////////////////////////////////////////////
{
 joedb::Memory_File file;
 test::Writable_Database db(file);

 const test::id_of_person joe = db.new_person("Joe", db.null_city());
 db.default_checkpoint();

 test::Readable readable(db);

 const std::string name = readable.get_string
 (
  test::interpreted_person::table_id,
  joe.get_record_id(),
  test::interpreted_person::name_field_id
 );

 EXPECT_EQ("Joe", name);
}
