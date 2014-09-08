#ifndef crazydb_parse_schema_declared
#define crazydb_parse_schema_declared

#include <iostream>

namespace crazydb
{
 struct Schema;

 bool parse_schema(std::istream &input, Schema &schema);
}

#endif
