#ifndef joedb_merge_declared
#define joedb_merge_declared

namespace joedb::interpreted
{
 class Database;
}

namespace joedb::ui
{
 void merge(interpreted::Database &merged, const interpreted::Database &db);
}

#endif
