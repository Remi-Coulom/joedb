#ifndef joedb_Field_declared
#define joedb_Field_declared

#include <string>
#include <vector>
#include <cassert>

#include "Type.h"
#include "index_types.h"

namespace joedb
{
 class Field
 {
  private:
   std::string name;
   const Type type;

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   std::vector<type> type_id##_column;
   #include "TYPE_MACRO.h"
   #undef TYPE_MACRO

  public:
   Field(const std::string &name, const Type &type, size_t size):
    name(name),
    type(type)
   {
    resize(size);
   }

   const std::string &get_name() const {return name;}
   void set_name(const std::string &new_name) {name = new_name;}
   const Type &get_type() const {return type;}

   void resize(size_t size)
   {
    switch (type.get_type_id())
    {
     case Type::type_id_t::null: break;
     #define TYPE_MACRO(type, return_type, type_id, R, W)\
     case Type::type_id_t::type_id: type_id##_column.resize(size); break;
     #include "TYPE_MACRO.h"
     #undef TYPE_MACRO
    }
   }

   #define TYPE_MACRO(T, return_type, type_id, R, W)\
   return_type get_##type_id(record_id_t record_id) const\
   {\
    assert(type.get_type_id() == Type::type_id_t::type_id);\
    return type_id##_column[record_id - 1];\
   }\
   void set_##type_id(record_id_t record_id, return_type value)\
   {\
    assert(type.get_type_id() == Type::type_id_t::type_id);\
    type_id##_column[record_id - 1] = value;\
   }
   #include "TYPE_MACRO.h"
   #undef TYPE_MACRO
 };
}

#endif
