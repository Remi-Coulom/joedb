#include "joedb/io/merge.h"
#include "joedb/interpreter/Database.h"

#include "gtest/gtest.h"

using namespace joedb;

/////////////////////////////////////////////////////////////////////////////
TEST(Merge_Test, merge_test)
/////////////////////////////////////////////////////////////////////////////
{
 const int databases = 4;
 Database db[databases];

 Table_Id person;
 Table_Id city;

 Field_Id person_name;
 Field_Id person_home;
 Field_Id city_name;

 for (int i = databases; --i >= 0;)
 {
  db[i].create_table("person");
  db[i].create_table("city");

  person = db[i].find_table("person");
  city = db[i].find_table("city");

  db[i].add_field(person, "name", Type::string());
  db[i].add_field(person, "home", Type::reference(city));
  db[i].add_field(city, "name", Type::string());

  person_name = db[i].find_field(person, "name");
  person_home = db[i].find_field(person, "home");
  city_name = db[i].find_field(city, "name");
 }

 db[1].insert_into(city, 1);
 db[1].update_string(city, 1, city_name, "Lille");
 db[1].insert_into(city, 2);
 db[1].update_string(city, 2, city_name, "Maubeuge");
 db[1].insert_into(person, 1);
 db[1].update_string(person, 1, person_name, "Toto");
 db[1].update_reference(person, 1, person_home, 1);

 db[2].insert_into(city, 1);
 db[2].update_string(city, 1, city_name, "Paris");
 db[2].insert_into(person, 1);
 db[2].update_string(person, 1, person_name, "Titi");
 db[2].update_reference(person, 1, person_home, 1);
 db[2].insert_into(person, 2);
 db[2].update_string(person, 2, person_name, "Tutu");
 db[2].update_reference(person, 2, person_home, 0);

 merge(db[0], db[1]);
 merge(db[0], db[2]);
 merge(db[0], db[3]);

 EXPECT_EQ(db[0].get_last_record_id(city), 3UL);
 EXPECT_EQ(db[0].get_last_record_id(person), 3UL);
 EXPECT_EQ(db[0].get_string(city, 1, city_name), "Lille");
 EXPECT_EQ(db[0].get_string(city, 3, city_name), "Paris");
 EXPECT_EQ(db[0].get_reference(person, 1, person_home), 1UL);
 EXPECT_EQ(db[0].get_reference(person, 2, person_home), 3UL);
 EXPECT_EQ(db[0].get_reference(person, 3, person_home), 0UL);
}
