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
    string,
    int32,
    int64,
    reference,
    boolean
   };

   enum {type_ids = int(type_id_t::boolean) + 1};

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
 };
}

#endif
