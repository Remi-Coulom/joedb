#include "tutorial.h"
#include <iostream>

int main()
{
 tutorial::File_Database db("tutorial.joedb");

 for (auto city: db.get_city_table())
  std::cout << db.get_name(city) << '\n';

 return 0;
}
