#ifndef crazydb_Schema_declared
#define crazydb_Schema_declared

#include "Table.h"
#include "Table_type.h"

namespace crazydb
{
 struct Schema
 {
  std::string name;
  std::list<Table> tables;
  std::list<Table_type> table_types;
 };
}

#endif
