#ifndef joedb_DB_Writeable_declared
#define joedb_DB_Writeable_declared

#include "Schema_Writeable.h"

namespace joedb
{
 class DB_Writeable: public Schema_Writeable
 {
  public:
   DB_Writeable(Database &db): Schema_Writeable(db) {}

   void comment(const std::string &comment) override
   {
    db.comment(comment);
   }

   void timestamp(int64_t timestamp) override
   {
    db.timestamp(timestamp);
   }

   void valid_data() override
   {
    db.valid_data();
   }

   void insert(table_id_t table_id, record_id_t record_id) override
   {
    db.insert_into(table_id, record_id);
   }

   void insert_vector(table_id_t table_id,
                      record_id_t record_id,
                      record_id_t size) override
   {
    db.insert_vector(table_id, record_id, size);
   }

   void delete_record(table_id_t table_id, record_id_t record_id) override
   {
    db.delete_from(table_id, record_id);
   }

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   void update_##type_id(table_id_t table_id,\
                         record_id_t record_id,\
                         field_id_t field_id,\
                         return_type value) override\
   {\
    db.update_##type_id(table_id, record_id, field_id, value);\
   }\
   void update_vector_##type_id(table_id_t table_id,\
                                record_id_t record_id,\
                                field_id_t field_id,\
                                record_id_t size,\
                                const type *value) override\
   {\
    db.update_vector_##type_id(table_id, record_id, field_id, size, value);\
   }
   #include "joedb/TYPE_MACRO.h"
   #undef TYPE_MACRO
 };
}

#endif
