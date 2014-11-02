#include "Database.h"
#include "dump.h"

#include <iostream>

using namespace crazydb;

int main()
{
 Database database;

 // TODO: use table_id_t instead of reference, and update via database

 Table &city = database.create_table("City");
 field_id_t city_name = city.add_field("name", Type::string());

 record_id_t paris = city.insert_record();
 city.update(paris, city_name, Value("Paris"));

 record_id_t lille = city.insert_record();
 city.update(lille, city_name, Value("Lille"));

 Table &person = database.create_table("Person");
 field_id_t person_name = person.add_field("name", Type::string());
 field_id_t person_dummy = person.add_field("dummy", Type::int32());
 field_id_t person_city = person.add_field("city", Type::reference("City"));

 record_id_t remi = person.insert_record();
 person.update(remi, person_name, Value("Rémi"));
 person.update(remi, person_city, Value(lille));

 record_id_t norbert = person.insert_record();
 person.update(norbert, person_name, Value("Norbert"));
 person.update(norbert, person_city, Value(lille));

// person.delete_record(remi);
 person.drop_field(person_dummy);

 record_id_t alex = person.insert_record();
 person.update(alex, person_name, Value("Alexandre"));
 person.update(alex, person_city, Value(paris));

 dump(std::cout, database);

 person.add_field("dummy", Type::int32());
 person.drop_field(person_city);

 dump(std::cout, database);

 return 0;
}
