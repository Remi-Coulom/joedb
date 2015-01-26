#ifndef joedb_Listener_declared
#define joedb_Listener_declared

#include "Type.h"

#include <string>

namespace joedb
{
 class Listener
 {
  public:
   virtual bool is_good() const {return true;}

   virtual void after_create_table(const std::string &name) {}
   virtual void after_drop_table(table_id_t table_id) {}

   virtual void after_add_field(table_id_t table_id,
                                const std::string &name,
                                Type type) {}
   virtual void after_drop_field(table_id_t table_id,
                                 field_id_t field_id) {}

   virtual void after_insert(table_id_t table_id, record_id_t record_id) {}
   virtual void after_delete(table_id_t table_id, record_id_t record_id) {}

   virtual void after_update_string(table_id_t table_id,
                                    record_id_t record_id,
                                    field_id_t field_id,
                                    const std::string &value) {}
   virtual void after_update_int32(table_id_t table_id,
                                   record_id_t record_id,
                                   field_id_t field_id,
                                   int32_t value) {}
   virtual void after_update_int64(table_id_t table_id,
                                   record_id_t record_id,
                                   field_id_t field_id,
                                   int64_t value) {}
   virtual void after_update_reference(table_id_t table_id,
                                       record_id_t record_id,
                                       field_id_t field_id,
                                       record_id_t value) {}
 };
}

#endif
