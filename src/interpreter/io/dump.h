#ifndef joedb_dump_declared
#define joedb_dump_declared

#include <iosfwd>

#include "Type.h"

namespace joedb
{
 class Database;

 void write_type(std::ostream &out, const Database &db, Type type);
 void dump(std::ostream &out, const Database &db);
}

#endif
