#ifndef joedb_Field_declared
#define joedb_Field_declared

#include <string>
#include <vector>
#include <algorithm>

#include "joedb/Exception.h"
#include "joedb/Type.h"
#include "joedb/index_types.h"

namespace joedb
{
 class Field
 {
  private:
   const Type type;

   #define TYPE_MACRO(cpp_type, return_type, type_id, R, W)\
   std::vector<cpp_type> type_id##_column;
   #include "joedb/TYPE_MACRO.h"

  public:
   Field(const Type &type, size_t size):
    type(type)
   {
    resize(size);
   }

   const Type &get_type() const {return type;}

   void resize(size_t size)
   {
    switch (type.get_type_id())
    {
     case Type::Type_Id::null: break;
     #define TYPE_MACRO(cpp_type, return_type, type_id, R, W)\
     case Type::Type_Id::type_id: type_id##_column.resize(size); break;
     #include "joedb/TYPE_MACRO.h"
    }
   }

   #define TYPE_MACRO(cpp_type, return_type, type_id, R, W)\
   return_type get_##type_id(Record_Id record_id) const\
   {\
    if (type.get_type_id() != Type::Type_Id::type_id)\
     throw Exception("type error");\
    return type_id##_column[to_underlying(record_id) - 1];\
   }\
   void set_##type_id(Record_Id record_id, return_type value)\
   {\
    if (type.get_type_id() != Type::Type_Id::type_id)\
     throw Exception("type error");\
    type_id##_column[to_underlying(record_id) - 1] = value;\
   }\
   const cpp_type *get_vector_##type_id() const\
   {\
    if (type.get_type_id() != Type::Type_Id::type_id)\
     throw Exception("type error");\
    return &type_id##_column[0];\
   }\
   void set_vector_##type_id(Record_Id record_id,\
                             Size size,\
                             const cpp_type *value)\
   {\
    if (type.get_type_id() != Type::Type_Id::type_id)\
     throw Exception("type error");\
    cpp_type *target = &type_id##_column[to_underlying(record_id) - 1];\
    if (target != value)\
     std::copy_n(value, size, target);\
   }\
   cpp_type *get_own_##type_id##_storage(Record_Id record_id)\
   {\
    if (type.get_type_id() != Type::Type_Id::type_id)\
     throw Exception("type error");\
    return &type_id##_column[to_underlying(record_id) - 1];\
   }\
   const cpp_type *get_own_##type_id##_storage(Record_Id record_id) const\
   {\
    if (type.get_type_id() != Type::Type_Id::type_id)\
     throw Exception("type error");\
    return &type_id##_column[to_underlying(record_id) - 1];\
   }
   #include "joedb/TYPE_MACRO.h"
 };
}

#endif
