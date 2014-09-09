#ifndef crazydb_Field_declared
#define crazydb_Field_declared

#include "Type.h"

namespace crazydb
{
 struct Field
 {
  std::string name;
  std::string type_string;
  Type type;
 };
}

#endif
