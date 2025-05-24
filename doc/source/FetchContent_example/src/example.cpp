#include "population/File_Client.h"

#include <iostream>

int main()
{
 population::File_Client client("population.joedb");

 const auto person = client.transaction([](population::Writable_Database &db)
 {
  return db.new_person("Joe");
 });

 std::cout << "Hello " << client.get_database().get_name(person) << "!\n";

 return 0;
}
