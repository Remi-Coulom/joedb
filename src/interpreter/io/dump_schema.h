#ifndef crazydb_dump_schema_declared
#define crazydb_dump_schema_declared

#include <iosfwd>

namespace crazydb
{
 class Database;

 void dump_schema(std::ostream &out, const Database &database);
}

#endif
