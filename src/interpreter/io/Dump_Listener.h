#ifndef joedb_Dump_Listener_declared
#define joedb_Dump_Listener_declared

#include "Schema_Listener.h"
#include "dump.h"
#include "string_io.h"

#include <iostream>

namespace joedb
{
 class Dump_Listener: public Schema_Listener
 {
  private:
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

  public:
   Dump_Listener(std::ostream &out): Schema_Listener(db), out(out) {}

   void after_create_table(const std::string &name) override
   {
    Schema_Listener::after_create_table(name);
    out << "create_table " << name << '\n';
   }

   void after_drop_table(table_id_t table_id) override
   {
    out << "drop_table " << get_table_name(table_id) << '\n';
    Schema_Listener::after_drop_table(table_id);
   }

   void after_add_field(table_id_t table_id,
                        const std::string &name,
                        Type type) override
   {
    Schema_Listener::after_add_field(table_id, name, type);
    out << "add_field " << get_table_name(table_id) << ' ' << name << ' ';
    write_type(out, db, type);
    out << '\n';
   }

   void after_drop_field(table_id_t table_id, field_id_t field_id) override
   {
    Schema_Listener::after_drop_field(table_id, field_id);
    out << "drop_field " << get_table_name(table_id) << ' ';
    out << get_field_name(table_id, field_id) << '\n';
   }

   void after_insert(table_id_t table_id, record_id_t record_id)
   {
    out << "insert_into " << get_table_name(table_id) << ' ';
    out << record_id << '\n';
   }

   void after_delete(table_id_t table_id, record_id_t record_id)
   {
    out << "delete_from " << get_table_name(table_id) << ' ';
    out << record_id << '\n';
   }

   void after_update_string(table_id_t table_id,
                            record_id_t record_id,
                            field_id_t field_id,
                            const std::string &value) override
   {
    out << "update " << get_table_name(table_id) << ' ';
    out << record_id << ' ';
    out << get_field_name(table_id, field_id) << ' ';
    joedb::write_string(out, value);
    out << '\n';
   }

#define AFTER_UPDATE(return_type, type_id)\
   void after_update_##type_id(table_id_t table_id,\
                               record_id_t record_id,\
                               field_id_t field_id,\
                               return_type value) override\
   {\
    out << "update " << get_table_name(table_id) << ' ';\
    out << record_id << ' ';\
    out << get_field_name(table_id, field_id) << ' ';\
    out << value << '\n';\
   }

   AFTER_UPDATE(int32_t, int32)
   AFTER_UPDATE(int64_t, int64)
   AFTER_UPDATE(record_id_t, reference)
   AFTER_UPDATE(bool, boolean)

#undef AFTER_UPDATE
 };
}

#endif
