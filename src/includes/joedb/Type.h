#ifndef joedb_Type_declared
#define joedb_Type_declared

#include "index_types.h"

namespace joedb
{
 typedef uint8_t Type_Id_Storage;

 class Type
 {
  public:
   enum class Type_Id: uint8_t
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
   Type_Id type_id;
   Table_Id table_id;

   Type(Type_Id type_id,
        Table_Id table_id):
    type_id(type_id),
    table_id(table_id)
   {}

  public:
   Type_Id get_type_id() const {return type_id;}
   Table_Id get_table_id() const {return table_id;}

   Type(): type_id(Type_Id::null) {}
   Type(Type_Id type_id): type_id(type_id) {}

   #define TYPE_MACRO(type, return_type, type_id, read, write)\
   static Type type_id() {return Type(Type_Id::type_id);};
   #define TYPE_MACRO_NO_REFERENCE
   #include "TYPE_MACRO.h"
   #undef TYPE_MACRO_NO_REFERENCE
   #undef TYPE_MACRO

   static Type reference(Table_Id table_id)
   {
    return Type(Type_Id::reference, table_id);
   }
 };
}

#endif
