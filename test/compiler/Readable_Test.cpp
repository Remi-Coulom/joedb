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
 db.soft_checkpoint();

 test::Readable readable(db);

 const std::string name = readable.get_string
 (
  test::person_table::id,
  joe.get_record_id(),
  test::person_table::name_field::id
 );

 EXPECT_EQ("Joe", name);
}
