#ifndef joedb_merge_declared
#define joedb_merge_declared

namespace joedb
{
 namespace interpreter {class Database;}
 void merge(interpreter::Database &merged, const interpreter::Database &db);
}

#endif
