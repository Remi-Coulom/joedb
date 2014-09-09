#ifndef crazydb_parse_schema_declared
#define crazydb_parse_schema_declared

#include <iostream>

namespace crazydb
{
 struct Schema;

 // Return true if OK, false if error
 bool parse_schema(std::istream &in, Schema &schema);

 // Reverse operation
 void write_schema(std::ostream &out, const Schema &schema);

 // Second pass after parse_schema
 bool parse_types(Schema &schema);
}

#endif
