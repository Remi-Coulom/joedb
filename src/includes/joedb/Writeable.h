#ifndef joedb_Writeable_declared
#define joedb_Writeable_declared

#include "Type.h"

#include <string>

namespace joedb
{
 class Writeable
 {
  public:
   virtual void create_table(const std::string &name) = 0;
   virtual void drop_table(table_id_t table_id) = 0;
   virtual void rename_table(table_id_t table_id, const std::string &name) = 0;

   virtual void add_field(table_id_t table_id,
                          const std::string &name,
                          Type type) = 0;
   virtual void drop_field(table_id_t table_id, field_id_t field_id) = 0;
   virtual void rename_field(table_id_t table_id,
                             field_id_t field_id,
                             const std::string &name) = 0;

   virtual void custom(const std::string &name) = 0;

   virtual void comment(const std::string &comment) = 0;
   virtual void timestamp(int64_t timestamp) = 0;
   virtual void valid_data() = 0;

   virtual void insert(table_id_t table_id, record_id_t record_id) = 0;
   virtual void insert_vector(table_id_t table_id,
                              record_id_t record_id,
                              record_id_t size) = 0;
   virtual void delete_record(table_id_t table_id, record_id_t record_id) = 0;

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   virtual void update_##type_id(table_id_t table_id,\
                                 record_id_t record_id,\
                                 field_id_t field_id,\
                                 return_type value) = 0;\
   virtual void update_vector_##type_id(table_id_t table_id,\
                                        record_id_t record_id,\
                                        field_id_t field_id,\
                                        record_id_t size,\
                                        const type *value);
   #include "TYPE_MACRO.h"
   #undef TYPE_MACRO

   virtual ~Writeable() {}
 };
}

#endif
