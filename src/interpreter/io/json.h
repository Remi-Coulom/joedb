#ifndef joedb_json_declared
#define joedb_json_declared

#include <iosfwd>

namespace joedb
{
 class Database;

 void write_json(std::ostream &out, const Database &db);
}

#endif
