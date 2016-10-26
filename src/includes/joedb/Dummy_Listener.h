#ifndef joedb_Dummy_Listener_declared
#define joedb_Dummy_Listener_declared

#include "Listener.h"

namespace joedb
{
 class Dummy_Listener: public Listener
 {
  public:
   void after_create_table(const std::string &name) override {};
   void after_drop_table(table_id_t table_id) override {};

   void after_add_field(table_id_t table_id,
                        const std::string &name,
                        Type type) override {};
   void after_drop_field(table_id_t table_id,
                         field_id_t field_id) override {};

   void after_custom(const std::string &name) override {};

   void after_insert(table_id_t table_id, record_id_t record_id) override {};
   void after_delete(table_id_t table_id, record_id_t record_id) override {};

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   void after_update_##type_id(table_id_t table_id,\
                               record_id_t record_id,\
                               field_id_t field_id,\
                               return_type value) override {};
   #include "TYPE_MACRO.h"
   #undef TYPE_MACRO
 };
}

#endif
