#ifndef crazydb_Table_declared
#define crazydb_Table_declared

#include <list>

#include "Field.h"

namespace crazydb
{
 struct Table
 {
  std::string name;
  std::list<Field> fields;
 };
}

#endif
