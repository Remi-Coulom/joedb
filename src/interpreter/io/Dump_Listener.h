#ifndef joedb_Dump_Listener_declared
#define joedb_Dump_Listener_declared

#include "Schema_Listener.h"
#include "type_io.h"

#include <iostream>

namespace joedb
{
 class Dump_Listener: public Schema_Listener
 {
  protected:
   Database db;
   std::ostream &out;

   std::string get_table_name(table_id_t table_id)
   {
    auto it = db.get_tables().find(table_id);
    if (it != db.get_tables().end())
     return it->second.get_name();
    else
     return "";
   }

   std::string get_field_name(table_id_t table_id, field_id_t field_id)
   {
    auto table = db.get_tables().find(table_id);
    if (table == db.get_tables().end())
     return "";

    auto field = table->second.get_fields().find(field_id);
    if (field == table->second.get_fields().end())
     return "";

    return field->second.get_name();
   }

   Type get_field_type(table_id_t table_id, field_id_t field_id)
   {
    auto table = db.get_tables().find(table_id);
    if (table == db.get_tables().end())
     return Type();

    auto field = table->second.get_fields().find(field_id);
    if (field == table->second.get_fields().end())
     return Type();

    return field->second.get_type();
   }

  public:
   Dump_Listener(std::ostream &out): Schema_Listener(db), out(out) {}
 };
}

#endif
