#ifndef joedb_Selective_Listener_declared
#define joedb_Selective_Listener_declared

#include "Listener.h"

namespace joedb
{
 class Selective_Listener: public Listener
 {
  public:
   enum Mode
   {
    schema = 1,
    data = 2
   };

  private:
   Listener &listener;
   const Mode mode;

  public:

   Selective_Listener(Listener &listener, Mode mode):
    listener(listener),
    mode(mode)
   {
   }

   bool is_good() const {return listener.is_good();}

   //
   // schema events
   //
   void after_create_table(const std::string &name) override
   {
    if (mode & schema)
     listener.after_create_table(name);
   }

   void after_drop_table(table_id_t table_id) override
   {
    if (mode & schema)
     listener.after_drop_table(table_id);
   }

   void after_add_field(table_id_t table_id,
                        const std::string &name,
                        Type type) override
   {
    if (mode & schema)
     listener.after_add_field(table_id, name, type);
   }

   void after_drop_field(table_id_t table_id,
                         field_id_t field_id) override
   {
    if (mode & schema)
     listener.after_drop_field(table_id, field_id);
   }

   //
   // data events
   //
   void after_insert(table_id_t table_id, record_id_t record_id) override
   {
    if (mode & data)
     listener.after_insert(table_id, record_id);
   }

   void after_delete(table_id_t table_id, record_id_t record_id) override
   {
    if (mode & data)
     listener.after_delete(table_id, record_id);
   }

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   void after_update_##type_id(table_id_t table_id,\
                               record_id_t record_id,\
                               field_id_t field_id,\
                               return_type value) override\
   {\
    if (mode & data)\
     listener.after_update_##type_id(table_id, record_id, field_id, value);\
   }
   #include "TYPE_MACRO.h"
   #undef TYPE_MACRO
 };
}

#endif
