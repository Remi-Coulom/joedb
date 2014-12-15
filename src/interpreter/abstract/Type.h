#ifndef joedb_Type_declared
#define joedb_Type_declared

#include <string>

#include "index_types.h"

namespace joedb
{
 class Type
 {
  public:
   enum Kind {null_id, string_id, int32_id, int64_id, reference_id};

  private:
   Kind kind;
   table_id_t table_id;

   Type(Kind kind): kind(kind) {}
   Type(Kind kind, table_id_t table_id): kind(kind), table_id(table_id) {}

  public:
   Kind get_kind() const {return kind;}
   table_id_t get_table_id() const {return table_id;}

   Type(): kind(null_id) {}
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
