#ifndef joedb_Type_declared
#define joedb_Type_declared

#include <string>

#include "index_types.h"

namespace joedb
{
 class Type
 {
  public:
   enum type_id_t {null_id, string_id, int32_id, int64_id, reference_id};

  private:
   type_id_t type_id;
   table_id_t table_id;

   Type(type_id_t type_id): type_id(type_id) {}
   Type(type_id_t type_id,
        table_id_t table_id):
    type_id(type_id),
    table_id(table_id)
   {}

  public:
   type_id_t get_type_id() const {return type_id;}
   table_id_t get_table_id() const {return table_id;}

   Type(): type_id(null_id) {}
   static Type string() {return Type(string_id);}
   static Type int32() {return Type(int32_id);}
   static Type int64() {return Type(int64_id);}
   static Type reference(table_id_t table_id)
   {
    return Type(reference_id, table_id);
   }
 };
}

#endif
