#include "joedb/Writable.h"

namespace joedb
{
 #define TYPE_MACRO(type, return_type, type_id, R, W)\
 void Writable::update_vector_##type_id\
 (\
  Table_Id table_id,\
  Record_Id record_id,\
  Field_Id field_id,\
  size_t size,\
  const type *value\
 )\
 {\
  for (size_t i = 0; i < size; i++)\
   update_##type_id(table_id, record_id + i, field_id, value[i]);\
 }
 #include "joedb/TYPE_MACRO.h"

 #define TYPE_MACRO(type, return_type, type_id, R, W)\
 void Writable::update_##type_id\
 (\
  Table_Id table_id,\
  Record_Id record_id,\
  Field_Id field_id,\
  return_type value\
 )\
 {\
 }
 #include "joedb/TYPE_MACRO.h"

 void Writable::default_checkpoint()
 {
  checkpoint(default_commit_level);
 }
}
