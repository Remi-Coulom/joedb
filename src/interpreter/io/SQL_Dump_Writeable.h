#ifndef joedb_SQL_Dump_Writeable_declared
#define joedb_SQL_Dump_Writeable_declared

#include "Dump_Writeable.h"
#include "type_io.h"

#include <iostream>

namespace joedb
{
 class SQL_Dump_Writeable: public Dump_Writeable
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

   std::string id_field_name = "\"__id\"";

  public:
   SQL_Dump_Writeable(std::ostream &out): Dump_Writeable(out) {}

   void create_table(const std::string &name) override
   {
    out << "CREATE TABLE \"" << name << "\"(" << id_field_name << " INTEGER PRIMARY KEY);\n";
    Schema_Writeable::create_table(name);
   }

   void drop_table(table_id_t table_id) override
   {
    out << "DROP TABLE \"" << get_table_name(table_id) << "\";\n";
    Schema_Writeable::drop_table(table_id);
   }

   void rename_table(table_id_t table_id,
                     const std::string &name) override
   {
    out << "ALTER TABLE \"" << get_table_name(table_id);
    out << "\" RENAME TO \"" << name << "\";\n";
    Schema_Writeable::rename_table(table_id, name);
   }

   void add_field(table_id_t table_id,
                  const std::string &name,
                  Type type) override
   {
    out << "ALTER TABLE \"" << get_table_name(table_id);
    out << "\" ADD \"" << name << "\" ";
    write_type(type);
    out << ";\n";
    Schema_Writeable::add_field(table_id, name, type);
   }

   void drop_field(table_id_t table_id, field_id_t field_id) override
   {
    out << "ALTER TABLE \"" << get_table_name(table_id);
    out << "\" DROP \"" << get_field_name(table_id, field_id) << "\";\n";
    Schema_Writeable::drop_field(table_id, field_id);
   }

   void rename_field(table_id_t table_id,
                     field_id_t field_id,
                     const std::string &name) override
   {
    out << "ALTER TABLE \"" << get_table_name(table_id) << "\" RENAME COLUMN \"";
    out << get_field_name(table_id, field_id) << "\" TO \"" << name << "\";\n";
    Schema_Writeable::rename_field(table_id, field_id, name);
   }

   void custom(const std::string &name) override
   {
    out << "-- custom: " << name << '\n';
   }

   void comment(const std::string &comment) override
   {
    out << "-- " << comment << '\n';
   }

   void timestamp(int64_t timestamp) override
   {
    out << "-- " << get_local_time(timestamp) << '\n';
   }

   void valid_data() override
   {
    out << "-- valid data\n";
   }

   void insert_into(table_id_t table_id, record_id_t record_id) override
   {
    out << "INSERT INTO \"" << get_table_name(table_id);
    out << "\"(" << id_field_name << ") VALUES(" << record_id << ");\n";
   }

   void insert_vector(table_id_t table_id,
                      record_id_t record_id,
                      record_id_t size) override
   {
    for (record_id_t i = 0; i < size; i++)
     insert_into(table_id, record_id + i);
   }

   void delete_from(table_id_t table_id, record_id_t record_id) override
   {
    out << "DELETE FROM \"" << get_table_name(table_id);
    out << "\" WHERE " << id_field_name << " = " << record_id << ";\n";
   }

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   void update_##type_id(table_id_t table_id,\
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
   #include "joedb/TYPE_MACRO.h"
   #undef TYPE_MACRO_NO_STRING
   #undef TYPE_MACRO

   void update_string(table_id_t table_id,
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
