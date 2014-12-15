#include "Database.h"
#include "dump.h"

#include <iostream>

using namespace joedb;

int main()
{
 Database db;

 table_id_t city = db.create_table("city");
 field_id_t city_name = db.add_field(city, "name", Type::string());

 record_id_t paris = db.insert_into(city);
 db.update(city, paris, city_name, Value("Paris"));

 record_id_t lille = db.insert_into(city);
 db.update(city, lille, city_name, Value("Lille"));

 table_id_t person = db.create_table("person");
 field_id_t person_name = db.add_field(person, "name", Type::string());
 field_id_t person_dummy = db.add_field(person, "dummy", Type::int32());
 field_id_t person_city = db.add_field(person, "city", Type::reference(city));

 record_id_t remi = db.insert_into(person);
 db.update(person, remi, person_name, Value("Rémi"));
 db.update(person, remi, person_city, Value(lille));

 record_id_t norbert = db.insert_into(person);
 db.update(person, norbert, person_name, Value("Norbert"));
 db.update(person, norbert, person_city, Value(lille));

// db.delete_record(person, remi);
 db.drop_field(person, person_dummy);

 record_id_t alex = db.insert_into(person);
 db.update(person, alex, person_name, Value("Alexandre"));
 db.update(person, alex, person_city, Value(paris));

 dump(std::cout, db);

 db.add_field(person, "dummy", Type::int32());
// db.drop_field(person, person_city);
 db.drop_table(city);

 dump(std::cout, db);


 return 0;
}
