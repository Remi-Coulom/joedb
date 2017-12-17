#include "merge.h"
#include "Database.h"

#include "gtest/gtest.h"

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
TEST(Merge_Test, merge_test)
/////////////////////////////////////////////////////////////////////////////
{
 const int databases = 3;
 Database db[databases];

 for (int i = databases; --i >= 0;)
 {
  db[i].create_table("person");
  db[i].create_table("city");

  const Table_Id person = db[i].find_table("person");
  const Table_Id city = db[i].find_table("city");

  db[i].add_field(person, "name", Type::string());
  db[i].add_field(person, "home", Type::reference(city));
  db[i].add_field(city, "name", Type::string());
 }

 merge(db[0], db[1]);
 merge(db[0], db[2]);

 EXPECT_EQ(0, 0);
}
