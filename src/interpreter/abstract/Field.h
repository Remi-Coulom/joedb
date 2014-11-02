#ifndef crazydb_Field_declared
#define crazydb_Field_declared

#include <string>

#include "Type.h"
#include "index_types.h"

namespace crazydb
{
 struct Field
 {
  field_id_t index;
  std::string name;
  Type type;
 };
}

#endif
