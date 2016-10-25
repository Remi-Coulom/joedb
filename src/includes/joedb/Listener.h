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

   virtual void after_create_table(const std::string &name) = 0;
   virtual void after_drop_table(table_id_t table_id) = 0;

   virtual void after_add_field(table_id_t table_id,
                                const std::string &name,
                                Type type) = 0;
   virtual void after_drop_field(table_id_t table_id,
                                 field_id_t field_id) = 0;

   virtual void after_insert(table_id_t table_id, record_id_t record_id) = 0;
   virtual void after_delete(table_id_t table_id, record_id_t record_id) = 0;

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   virtual void after_update_##type_id(table_id_t table_id,\
                                       record_id_t record_id,\
                                       field_id_t field_id,\
                                       return_type value) = 0;
   #include "TYPE_MACRO.h"
   #undef TYPE_MACRO

   virtual ~Listener() {}
 };
}

#endif
