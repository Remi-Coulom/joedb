#include "Database.h"
#include "dump_schema.h"

#include <iostream>

using namespace crazydb;

int main()
{
 Database database;

 Table &city = database.create_table("City");
 city.add_field("name", Type::string());

 Table &person = database.create_table("Person");
 person.add_field("name", Type::string());
 person.add_field("city", Type::reference("City"));

 dump_schema(std::cout, database);

 return 0;
}
