#ifndef joedb_Readable_Multiplexer_declared
#define joedb_Readable_Multiplexer_declared

#include "Multiplexer.h"
#include "Readable_Writeable.h"

namespace joedb
{
 class Readable_Multiplexer: public Readable_Writeable, public Multiplexer
 {
  private:
   Readable &readable;

  public:
   Readable_Multiplexer(Readable_Writeable &readable_writeable):
    readable(readable_writeable)
   {
    add_writeable(readable_writeable);
   }

   const std::map<table_id_t, std::string> &get_tables() const override
   {
    return readable.get_tables();
   }

   const std::map<table_id_t, std::string> &get_fields(table_id_t table_id) const
   {
    return readable.get_fields(table_id);
   }

   const Type &get_field_type(table_id_t table_id,
                              field_id_t field_id) const
   {
    return readable.get_field_type(table_id, field_id);
   }

   record_id_t get_last_record_id(table_id_t table_id) const
   {
    return readable.get_last_record_id(table_id);
   }

   bool is_used(table_id_t table_id, record_id_t record_id) const
   {
    return readable.is_used(table_id, record_id);
   }

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   return_type get_##type_id(table_id_t table_id,\
                             record_id_t record_id,\
                             field_id_t field_id) const\
   {\
    return readable.get_##type_id(table_id, record_id, field_id);\
   }
   #include "TYPE_MACRO.h"
   #undef TYPE_MACRO
 };
}

#endif
