#include "db/test.h"

#include "gtest/gtest.h"

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, Pullable_Database)
/////////////////////////////////////////////////////////////////////////////
{
 const char * const file_name = "compiler_test.joedb";
 std::remove(file_name);

 {
  my_namespace::is_nested::test::File_Database db
  (
   file_name,
   joedb::Open_Mode::create_new,
   joedb::Readonly_Journal::Check::all,
   joedb::Commit_Level::no_commit
  );

  db.new_city("Paris");
  db.checkpoint();

  joedb::File file(file_name, joedb::Open_Mode::read_existing);
  my_namespace::is_nested::test::Pullable_Database pullable_db(file);

  EXPECT_FALSE(pullable_db.pull());
  EXPECT_EQ(pullable_db.get_city_table().get_size(), 1UL);

  db.new_city("Tokyo");
  db.checkpoint();

  EXPECT_EQ(pullable_db.get_city_table().get_size(), 1UL);
  EXPECT_TRUE(pullable_db.pull());
  EXPECT_EQ(pullable_db.get_city_table().get_size(), 2UL);
 }

 std::remove(file_name);
}
