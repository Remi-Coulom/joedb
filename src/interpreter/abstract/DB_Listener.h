#ifndef joedb_DB_Listener_declared
#define joedb_DB_Listener_declared

#include "Schema_Listener.h"

namespace joedb
{
 class DB_Listener: public Schema_Listener
 {
  public:
   DB_Listener(Database &db): Schema_Listener(db) {}

   void after_comment(const std::string &comment) override
   {
    db.comment(comment);
   }

   void after_time_stamp(int64_t time_stamp) override
   {
    db.time_stamp(time_stamp);
   }

   void after_checkpoint() override
   {
    db.checkpoint();
   }

   void after_insert(table_id_t table_id, record_id_t record_id) override
   {
    error |= !db.insert_into(table_id, record_id);
   }

   void after_insert_vector(table_id_t table_id,
                            record_id_t record_id,
                            record_id_t size) override
   {
    error |= !db.insert_vector(table_id, record_id, size);
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
   }\
   void after_update_vector_##type_id(table_id_t table_id,\
                                      record_id_t record_id,\
                                      field_id_t field_id,\
                                      record_id_t size,\
                                      const type *value) override\
   {\
    error |= !db.update_vector_##type_id(table_id, record_id, field_id, size, value);\
   }
   #include "TYPE_MACRO.h"
   #undef TYPE_MACRO
 };
}

#endif
