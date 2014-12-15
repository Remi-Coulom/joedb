#ifndef joedb_dump_declared
#define joedb_dump_declared

#include <iosfwd>

namespace joedb
{
 class Database;

 void dump(std::ostream &out, const Database &database);
}

#endif
