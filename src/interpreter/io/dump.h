#ifndef joedb_dump_declared
#define joedb_dump_declared

namespace joedb
{
 class Database;
 class Listener;

 void dump(const Database &db, Listener &listener);
}

#endif
