#ifndef joedb_merge_declared
#define joedb_merge_declared

namespace joedb
{
 class Database;
}

namespace joedb::ui
{
 void merge(Database &merged, const Database &db);
}

#endif
