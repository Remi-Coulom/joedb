#ifndef joedb_Multiplexer_declared
#define joedb_Multiplexer_declared

#include <vector>

#include "Writeable.h"

namespace joedb
{
 class Multiplexer: public virtual Writeable
 {
  private:
   std::vector<Writeable *> writeables;

  public:
   void add_writeable(Writeable &writeable);

   void create_table(const std::string &name) override;
   void drop_table(table_id_t table_id) override;
   void rename_table(table_id_t table_id, const std::string &name) override;
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
