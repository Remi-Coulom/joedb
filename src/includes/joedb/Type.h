#ifndef joedb_Type_declared
#define joedb_Type_declared

#include "index_types.h"

namespace joedb
{
 class Type
 {
  public:
   enum class type_id_t: uint8_t
   {
    null,
    #define TYPE_MACRO(a, b, c, d, e) c,
    #include "TYPE_MACRO.h"
    #undef TYPE_MACRO
   };

   #define TYPE_MACRO(a, b, c, d, e) 1 +
   enum {type_ids =
    #include "TYPE_MACRO.h"
    1
   };
   #undef TYPE_MACRO

  private:
   type_id_t type_id;
   table_id_t table_id;

   Type(type_id_t type_id,
        table_id_t table_id):
    type_id(type_id),
    table_id(table_id)
   {}

  public:
   type_id_t get_type_id() const {return type_id;}
   table_id_t get_table_id() const {return table_id;}

   Type(): type_id(type_id_t::null) {}
   Type(type_id_t type_id): type_id(type_id) {}

   static Type string() {return Type(type_id_t::string);}
   static Type int32() {return Type(type_id_t::int32);}
   static Type int64() {return Type(type_id_t::int64);}
   static Type reference(table_id_t table_id)
   {
    return Type(type_id_t::reference, table_id);
   }
   static Type boolean() {return Type(type_id_t::boolean);}
   static Type float32() {return Type(type_id_t::float32);}
   static Type float64() {return Type(type_id_t::float64);}
   static Type int8() {return Type(type_id_t::int8);}
 };
}

#endif
