#include "Database.h"

using namespace crazydb;

int main()
{
 Database database;

 database.create_table("City");
 database.alter_table_add("City", "name", Type::string());

 database.create_table("Person");
 database.alter_table_add("Person", "name", Type::string());
 database.alter_table_add("Person", "city", Type::reference("City"));

 return 0;
}
