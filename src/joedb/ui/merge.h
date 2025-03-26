#ifndef joedb_merge_declared
#define joedb_merge_declared

namespace joedb::interpreter
{
 class Database;
}

namespace joedb::ui
{
 void merge(interpreter::Database &merged, const interpreter::Database &db);
}

#endif
