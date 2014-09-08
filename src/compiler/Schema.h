#ifndef crazydb_Schema_declared
#define crazydb_Schema_declared

#include "Table.h"

namespace crazydb
{
 struct Schema
 {
  std::string name;
  std::list<Table> tables;
 };
}

#endif
