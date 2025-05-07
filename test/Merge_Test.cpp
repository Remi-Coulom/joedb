#include "joedb/ui/merge.h"
#include "joedb/interpreted/Database.h"

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

 db[1].insert_into(city, Record_Id(0));
 db[1].update_string(city, Record_Id(0), city_name, "Lille");
 db[1].insert_into(city, Record_Id(2));
 db[1].update_string(city, Record_Id(2), city_name, "Maubeuge");
 db[1].insert_into(person, Record_Id(0));
 db[1].update_string(person, Record_Id(0), person_name, "Toto");
 db[1].update_reference(person, Record_Id(0), person_home, Record_Id(0));

 db[2].insert_into(city, Record_Id(0));
 db[2].update_string(city, Record_Id(0), city_name, "Paris");
 db[2].insert_into(person, Record_Id(0));
 db[2].update_string(person, Record_Id(0), person_name, "Titi");
 db[2].update_reference(person, Record_Id(0), person_home, Record_Id(0));
 db[2].insert_into(person, Record_Id(1));
 db[2].update_string(person, Record_Id(1), person_name, "Tutu");
 db[2].update_reference(person, Record_Id(1), person_home, null);

 merge(db[0], db[1]);
 merge(db[0], db[2]);
 merge(db[0], db[3]);

 EXPECT_EQ(db[0].get_size(city), Record_Id(4));
 EXPECT_EQ(db[0].get_size(person), Record_Id(3));
 EXPECT_EQ(db[0].get_string(city, Record_Id(0), city_name), "Lille");
 EXPECT_EQ(db[0].get_string(city, Record_Id(2), city_name), "Maubeuge");
 EXPECT_EQ(db[0].get_string(city, Record_Id(3), city_name), "Paris");
 EXPECT_EQ(db[0].get_reference(person, Record_Id(0), person_home), Record_Id(0));
 EXPECT_EQ(db[0].get_reference(person, Record_Id(1), person_home), Record_Id(3));
 EXPECT_EQ(db[0].get_reference(person, Record_Id(2), person_home), null);
}
