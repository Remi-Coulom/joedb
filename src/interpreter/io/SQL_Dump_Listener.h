#ifndef joedb_SQL_Dump_Listener_declared
#define joedb_SQL_Dump_Listener_declared

#include "Dump_Listener.h"

namespace joedb
{
 class SQL_Dump_Listener: public Dump_Listener
 {
  private:
   void write_type(Type type)
   {
    switch(type.get_type_id())
    {
     case Type::type_id_t::null:
      out << "NULL";
     break;

     case Type::type_id_t::string:
      out << "TEXT";
     break;

     case Type::type_id_t::int32:
      out << "INTEGER";
     break;

     case Type::type_id_t::int64:
      out << "BIGINT";
     break;

     case Type::type_id_t::reference:
     {
      out << "INTEGER REFERENCES ";
      table_id_t table_id = type.get_table_id();
      const auto it = db.get_tables().find(table_id);
      out << '\"';
      if (it != db.get_tables().end())
       out << it->second.get_name();
      else
       out << "a_deleted_table";
      out << '\"';
     }
     break;

     case Type::type_id_t::boolean:
      out << "SMALLINT";
     break;

     case Type::type_id_t::float32:
      out << "REAL";
     break;

     case Type::type_id_t::float64:
      out << "REAL";
     break;

     case Type::type_id_t::int8:
      out << "SMALLINT";
     break;

     case Type::type_id_t::int16:
      out << "SMALLINT";
     break;
    }
   }

   std::string id_field_name = "\"joedb_key\"";

  public:
   SQL_Dump_Listener(std::ostream &out): Dump_Listener(out) {}

   void after_create_table(const std::string &name) override
   {
    out << "CREATE TABLE \"" << name << "\"(" << id_field_name << " INTEGER PRIMARY KEY);\n";
    Schema_Listener::after_create_table(name);
   }

   void after_drop_table(table_id_t table_id) override
   {
    out << "DROP TABLE \"" << get_table_name(table_id) << "\";\n";
    Schema_Listener::after_drop_table(table_id);
   }

   void after_rename_table(table_id_t table_id,
                           const std::string &name) override
   {
    out << "ALTER TABLE \"" << get_table_name(table_id);
    out << "\" RENAME TO \"" << name << "\";\n";
    Schema_Listener::after_rename_table(table_id, name);
   }

   void after_add_field(table_id_t table_id,
                        const std::string &name,
                        Type type) override
   {
    out << "ALTER TABLE \"" << get_table_name(table_id);
    out << "\" ADD \"" << name << "\" ";
    write_type(type);
    out << ";\n";
    Schema_Listener::after_add_field(table_id, name, type);
   }

   void after_drop_field(table_id_t table_id, field_id_t field_id) override
   {
    out << "ALTER TABLE \"" << get_table_name(table_id);
    out << "\" DROP \"" << get_field_name(table_id, field_id) << "\";\n";
    Schema_Listener::after_drop_field(table_id, field_id);
   }

   void after_rename_field(table_id_t table_id,
                           field_id_t field_id,
                           const std::string &name) override
   {
    out << "ALTER TABLE \"" << get_table_name(table_id) << "\" RENAME COLUMN \"";
    out << get_field_name(table_id, field_id) << "\" TO \"" << name << "\";\n";
    Schema_Listener::after_rename_field(table_id, field_id, name);
   }

   void after_custom(const std::string &name) override
   {
    out << "-- custom: " << name << '\n';
   }

   void after_comment(const std::string &comment) override
   {
    out << "-- ";
    write_string(out, comment);
    out << '\n';
   }

   void after_time_stamp(int64_t time_stamp) override
   {
    out << "-- time_stamp " << time_stamp << '\n';
   }

   void after_insert(table_id_t table_id, record_id_t record_id) override
   {
    out << "INSERT INTO \"" << get_table_name(table_id);
    out << "\"(" << id_field_name << ") VALUES(" << record_id << ");\n";
   }

   void after_insert_vector(table_id_t table_id,
                            record_id_t record_id,
                            record_id_t size) override
   {
    for (record_id_t i = 0; i < size; i++)
     after_insert(table_id, record_id + i);
   }

   void after_delete(table_id_t table_id, record_id_t record_id) override
   {
    out << "DELETE FROM \"" << get_table_name(table_id);
    out << "\" WHERE " << id_field_name << " = " << record_id << ";\n";
   }

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   void after_update_##type_id(table_id_t table_id,\
                               record_id_t record_id,\
                               field_id_t field_id,\
                               return_type value) override\
   {\
    out << "UPDATE \"" << get_table_name(table_id);\
    out << "\" SET \"" << get_field_name(table_id, field_id) << "\" = ";\
    joedb::write_##type_id(out, value);\
    out << " WHERE " << id_field_name << " = " << record_id << ";\n";\
   }
   #define TYPE_MACRO_NO_STRING
   #include "TYPE_MACRO.h"
   #undef TYPE_MACRO_NO_STRING
   #undef TYPE_MACRO

   void after_update_string(table_id_t table_id,
                            record_id_t record_id,
                            field_id_t field_id,
                            const std::string &value) override
   {
    out << "UPDATE \"" << get_table_name(table_id);
    out << "\" SET \"" << get_field_name(table_id, field_id) << "\" = ";
    joedb::write_sql_string(out, value);
    out << " WHERE " << id_field_name << " = " << record_id << ";\n";
   }
 };
}

#endif
