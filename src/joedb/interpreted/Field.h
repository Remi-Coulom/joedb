#ifndef joedb_Field_declared
#define joedb_Field_declared

#include <vector>
#include <algorithm>
#include <variant>

#include "joedb/Type.h"
#include "joedb/index_types.h"

namespace joedb::interpreted
{
 class Field
 {
  private:
   const Type type;

   std::variant<std::monostate
   #define TYPE_MACRO(cpp_type, return_type, type_id, R, W)\
   , std::vector<cpp_type>
   #include "joedb/TYPE_MACRO.h"
   > column;

   static constexpr size_t index(Record_Id record_id)
   {
    return to_underlying(record_id) - 1;
   }

  public:
   Field(const Type &type, size_t size):
    type(type)
   {
    switch (type.get_type_id())
    {
     case Type::Type_Id::null: break;
     #define TYPE_MACRO(cpp_type, return_type, type_id, R, W)\
     case Type::Type_Id::type_id:\
      column.emplace<std::vector<cpp_type>>(size);\
     break;
     #include "joedb/TYPE_MACRO.h"
    }
   }

   const Type &get_type() const {return type;}

   void resize(size_t size)
   {
    switch (type.get_type_id())
    {
     case Type::Type_Id::null: break;
     #define TYPE_MACRO(cpp_type, return_type, type_id, R, W)\
     case Type::Type_Id::type_id:\
      std::get<std::vector<cpp_type>>(column).resize(size);\
     break;
     #include "joedb/TYPE_MACRO.h"
    }
   }

   #define TYPE_MACRO(cpp_type, return_type, type_id, R, W)\
   return_type get_##type_id(Record_Id record_id) const\
   {\
    return std::get<std::vector<cpp_type>>(column)[index(record_id)];\
   }\
   void set_##type_id(Record_Id record_id, return_type value)\
   {\
    std::get<std::vector<cpp_type>>(column)[index(record_id)] = value;\
   }\
   const cpp_type *get_vector_##type_id() const\
   {\
    return &std::get<std::vector<cpp_type>>(column)[0];\
   }\
   void set_vector_##type_id(Record_Id record_id,\
                             size_t size,\
                             const cpp_type *value)\
   {\
    cpp_type *target =\
      &std::get<std::vector<cpp_type>>(column)[index(record_id)];\
    if (target != value)\
     std::copy_n(value, size, target);\
   }\
   cpp_type *get_own_##type_id##_storage(Record_Id record_id)\
   {\
    return &std::get<std::vector<cpp_type>>(column)[index(record_id)];\
   }\
   const cpp_type *get_own_##type_id##_storage(Record_Id record_id) const\
   {\
    return &std::get<std::vector<cpp_type>>(column)[index(record_id)];\
   }
   #include "joedb/TYPE_MACRO.h"
 };
}

#endif
