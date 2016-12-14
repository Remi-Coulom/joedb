#ifndef joedb_Safe_Writeable_declared
#define joedb_Safe_Writeable_declared

#include "Writeable.h"
#include "Database.h"
#include "Journal_File.h"
#include "DB_Writeable.h"

namespace joedb
{
 class Safe_Writeable: public Writeable
 {
  private:
   Database db;
   DB_Writeable db_writeable;
   bool safe_insert;
   record_id_t max_record_id;

   bool is_existing_table_id(table_id_t table_id) const;

   bool is_update_ok(table_id_t table_id,
                     record_id_t record_id,
                     field_id_t field_id,
                     record_id_t size,
                     Type::type_id_t type_id) const;

  public:
   Safe_Writeable(record_id_t max_record_id = 0):
    db_writeable(db),
    safe_insert(true),
    max_record_id(max_record_id)
   {
   }

   const Database &get_db() const {return db;}

   void create_table(const std::string &name) override;
   void drop_table(table_id_t table_id) override;
   void rename_table(table_id_t table_id,
                           const std::string &name) override;

   void add_field(table_id_t table_id,
                  const std::string &name,
                  Type type) override;
   void drop_field(table_id_t table_id, field_id_t field_id) override;
   void rename_field(table_id_t table_id,
                     field_id_t field_id,
                     const std::string &name) override;

   void custom(const std::string &name) override;

   void comment(const std::string &comment) override;
   void timestamp(int64_t timestamp) override;
   void valid_data() override;

   void insert(table_id_t table_id, record_id_t record_id) override;
   void insert_vector(table_id_t table_id,
                      record_id_t record_id,
                      record_id_t size) override;
   void delete_record(table_id_t table_id, record_id_t record_id) override;

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   void update_##type_id(table_id_t table_id,\
                         record_id_t record_id,\
                         field_id_t field_id,\
                         return_type value) override;\
   void update_vector_##type_id(table_id_t table_id,\
                                record_id_t record_id,\
                                field_id_t field_id,\
                                record_id_t size,\
                                const type *value) override;

   #include "TYPE_MACRO.h"
   #undef TYPE_MACRO
 };
}

#endif
