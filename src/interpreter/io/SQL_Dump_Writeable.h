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
      out << key_type << " REFERENCES ";
      out << '\"' << db.get_table_name(type.get_table_id()) << '\"';
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
   std::string key_type = "INTEGER";

  public:
   SQL_Dump_Writeable(std::ostream &out): Dump_Writeable(out) {}

   void create_table(const std::string &name) override
   {
    out << "CREATE TABLE \"" << name << "\"(" << id_field_name << ' ' << key_type << " PRIMARY KEY);\n";
    Schema_Writeable::create_table(name);
   }

   void drop_table(table_id_t table_id) override
   {
    out << "DROP TABLE \"" << db.get_table_name(table_id) << "\";\n";
    Schema_Writeable::drop_table(table_id);
   }

   void rename_table(table_id_t table_id,
                     const std::string &name) override
   {
    out << "ALTER TABLE \"" << db.get_table_name(table_id);
    out << "\" RENAME TO \"" << name << "\";\n";
    Schema_Writeable::rename_table(table_id, name);
   }

   void add_field(table_id_t table_id,
                  const std::string &name,
                  Type type) override
   {
    out << "ALTER TABLE \"" << db.get_table_name(table_id);
    out << "\" ADD \"" << name << "\" ";
    write_type(type);
    out << ";\n";
    Schema_Writeable::add_field(table_id, name, type);
   }

   void drop_field(table_id_t table_id, field_id_t field_id) override
   {
    out << "ALTER TABLE \"" << db.get_table_name(table_id);
    out << "\" DROP \"" << db.get_field_name(table_id, field_id) << "\";\n";
    Schema_Writeable::drop_field(table_id, field_id);
   }

   void rename_field(table_id_t table_id,
                     field_id_t field_id,
                     const std::string &name) override
   {
    out << "ALTER TABLE \"" << db.get_table_name(table_id) << "\" RENAME COLUMN \"";
    out << db.get_field_name(table_id, field_id) << "\" TO \"" << name << "\";\n";
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

   record_id_t get_max_record_id() const override {return 0;}

   void insert_into(table_id_t table_id, record_id_t record_id) override
   {
    out << "INSERT INTO \"" << db.get_table_name(table_id);
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
    out << "DELETE FROM \"" << db.get_table_name(table_id);
    out << "\" WHERE " << id_field_name << " = " << record_id << ";\n";
   }

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   void update_##type_id(table_id_t table_id,\
                         record_id_t record_id,\
                         field_id_t field_id,\
                         return_type value) override\
   {\
    out << "UPDATE \"" << db.get_table_name(table_id);\
    out << "\" SET \"" << db.get_field_name(table_id, field_id) << "\" = ";\
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
    out << "UPDATE \"" << db.get_table_name(table_id);
    out << "\" SET \"" << db.get_field_name(table_id, field_id) << "\" = ";
    joedb::write_sql_string(out, value);
    out << " WHERE " << id_field_name << " = " << record_id << ";\n";
   }
 };
}

#endif
