#include <iostream>

#include "parse_schema.h"
#include "Schema.h"

using namespace crazydb;

int main()
{
 Schema schema;

 if (!parse_schema(std::cin, schema))
 {
  std::cout << "Error\n";
  return 1;
 }

 write_schema(std::cout, schema);

 return 0;
}
