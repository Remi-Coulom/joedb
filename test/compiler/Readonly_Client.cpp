#include "db/test/File_Database.h"
#include "db/test/Readonly_Client.h"

#include "gtest/gtest.h"

/////////////////////////////////////////////////////////////////////////////
TEST(Compiler, Readonly_Client)
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
  my_namespace::is_nested::test::Readonly_Client client(file);

  EXPECT_FALSE(client.pull());
  EXPECT_EQ(client.get_database().get_city_table().get_size(), 1UL);

  db.new_city("Tokyo");
  db.checkpoint();

  EXPECT_EQ(client.get_database().get_city_table().get_size(), 1UL);
  EXPECT_TRUE(client.pull());
  EXPECT_EQ(client.get_database().get_city_table().get_size(), 2UL);
 }

 std::remove(file_name);
}
