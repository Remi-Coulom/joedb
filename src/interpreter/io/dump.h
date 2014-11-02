#ifndef crazydb_dump_declared
#define crazydb_dump_declared

#include <iosfwd>

namespace crazydb
{
 class Database;

 void dump(std::ostream &out, const Database &database);
}

#endif
