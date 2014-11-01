#ifndef crazydb_dump_schema_declared
#define crazydb_dump_schema_declared

#include <iosfwd>

namespace crazydb
{
 class Schema;

 void dump_schema(std::ostream &out, const Schema &schema);
}

#endif
