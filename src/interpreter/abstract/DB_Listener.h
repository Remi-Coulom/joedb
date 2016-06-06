#ifndef joedb_DB_Listener_declared
#define joedb_DB_Listener_declared

#include "Schema_Listener.h"

namespace joedb
{
 class DB_Listener: public Schema_Listener
 {
  public:
   DB_Listener(Database &db): Schema_Listener(db) {}

   void after_insert(table_id_t table_id, record_id_t record_id) override
   {
    error |= !db.insert_into(table_id, record_id);
   }

   void after_delete(table_id_t table_id, record_id_t record_id) override
   {
    error |= !db.delete_from(table_id, record_id);
   }

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   void after_update_##type_id(table_id_t table_id,\
                               record_id_t record_id,\
                               field_id_t field_id,\
                               return_type value) override\
   {\
    error |= !db.update_##type_id(table_id, record_id, field_id, value);\
   }
   #include "TYPE_MACRO.h"
   #undef TYPE_MACRO
 };
}

#endif
