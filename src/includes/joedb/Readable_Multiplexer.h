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

   record_id_t get_max_record_id() const override
   {
    return readable.get_max_record_id();
   }

   const std::map<table_id_t, Table> &get_tables() const override
   {
    return readable.get_tables();
   }

   size_t get_current_table_id() const override
   {
    return readable.get_current_table_id();
   }

   table_id_t find_table(const std::string &name) const override
   {
    return readable.find_table(name);
   }

   field_id_t find_field(table_id_t table_id,
                         const std::string &name) const override
   {
    return readable.find_field(table_id, name);
   }

   Type::type_id_t get_field_type(table_id_t table_id,
                                  field_id_t field_id) const override
   {
    return readable.get_field_type(table_id, field_id);
   }

   const std::vector<std::string> &get_custom_names() const override
   {
    return readable.get_custom_names();
   }
 };
}

#endif
