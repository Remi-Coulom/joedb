#include "joedb/ui/write_value.h"
#include "joedb/ui/type_io.h"
#include "joedb/Readable.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void write_value
 ////////////////////////////////////////////////////////////////////////////
 (
  std::ostream &out,
  const Readable &readable,
  Table_Id table_id,
  Record_Id record_id,
  Field_Id field_id
 )
 {
  switch (readable.get_field_type(table_id, field_id).get_type_id())
  {
   case Type::Type_Id::null:
   break;

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   case Type::Type_Id::type_id:\
    write_##type_id(out, readable.get_##type_id(table_id, record_id, field_id));\
   break;
   #include "joedb/TYPE_MACRO.h"
  }
 }
}
