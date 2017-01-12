#include "Writeable.h"

namespace joedb
{
 #define TYPE_MACRO(type, return_type, type_id, R, W)\
 void Writeable::update_vector_##type_id(Table_Id table_id,\
                                        Record_Id record_id,\
                                        Field_Id field_id,\
                                        Record_Id size,\
                                        const type *value)\
 {\
  for (Record_Id i = 0; i < size; i++)\
   update_##type_id(table_id, record_id + i, field_id, value[i]);\
 }
 #include "TYPE_MACRO.h"
 #undef TYPE_MACRO
}
