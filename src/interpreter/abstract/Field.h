#ifndef joedb_Field_declared
#define joedb_Field_declared

#include <string>

#include "Type.h"
#include "index_types.h"

namespace joedb
{
 struct Field
 {
  field_id_t index;
  std::string name;
  Type type;
 };
}

#endif
