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
   const std::string name;
   const Type type;

   std::vector<std::string> string_column;
   std::vector<int32_t> int32_column;
   std::vector<int64_t> int64_column;
   std::vector<record_id_t> reference_column;

  public:
   Field(const std::string &name, const Type &type, size_t size):
    name(name),
    type(type)
   {
    resize(size);
   }

   const std::string &get_name() const {return name;}
   const Type &get_type() const {return type;}

   void resize(size_t size)
   {
    switch (type.get_type_id())
    {
     case Type::type_id_t::null:                                     break;
     case Type::type_id_t::string:    string_column.resize(size);    break;
     case Type::type_id_t::int32:     int32_column.resize(size);     break;
     case Type::type_id_t::int64:     int64_column.resize(size);     break;
     case Type::type_id_t::reference: reference_column.resize(size); break;
    }
   }

#define FIELD_GETSET(return_type, type_id)\
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

   FIELD_GETSET(const std::string &, string)
   FIELD_GETSET(int32_t, int32)
   FIELD_GETSET(int64_t, int64)
   FIELD_GETSET(record_id_t, reference)

#undef FIELD_GETSET
 };
}

#endif
