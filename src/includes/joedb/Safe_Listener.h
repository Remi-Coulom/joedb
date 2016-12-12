#ifndef joedb_Safe_Listener_declared
#define joedb_Safe_Listener_declared

#include "Listener.h"
#include "Database.h"
#include "Journal_File.h"
#include "DB_Listener.h"

namespace joedb
{
 class Safe_Listener: public Listener
 {
  private:
   Database db;
   DB_Listener db_listener;
   bool safe_insert;
   record_id_t max_record_id;

   bool is_existing_table_id(table_id_t table_id) const;

   bool is_update_ok(table_id_t table_id,
                     record_id_t record_id,
                     field_id_t field_id,
                     record_id_t size,
                     Type::type_id_t type_id) const;

  public:
   Safe_Listener(record_id_t max_record_id):
    db_listener(db),
    safe_insert(true),
    max_record_id(max_record_id)
   {
   }

   void after_create_table(const std::string &name) override;
   void after_drop_table(table_id_t table_id) override;
   void after_rename_table(table_id_t table_id,
                           const std::string &name) override;

   void after_add_field(table_id_t table_id,
                        const std::string &name,
                        Type type) override;
   void after_drop_field(table_id_t table_id,
                         field_id_t field_id) override;
   void after_rename_field(table_id_t table_id,
                           field_id_t field_id,
                           const std::string &name) override;

   void after_custom(const std::string &name) override;

   void after_comment(const std::string &comment) override;
   void after_timestamp(int64_t timestamp) override;
   void after_valid_data() override;

   void after_insert(table_id_t table_id, record_id_t record_id) override;
   void after_insert_vector(table_id_t table_id,
                            record_id_t record_id,
                            record_id_t size) override;
   void after_delete(table_id_t table_id, record_id_t record_id) override;

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   void after_update_##type_id(table_id_t table_id,\
                               record_id_t record_id,\
                               field_id_t field_id,\
                               return_type value) override;\
   void after_update_vector_##type_id(table_id_t table_id,\
                                      record_id_t record_id,\
                                      field_id_t field_id,\
                                      record_id_t size,\
                                      const type *value) override;

   #include "TYPE_MACRO.h"
   #undef TYPE_MACRO
 };
}

#endif
