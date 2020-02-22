#ifndef joedb_SQL_Dump_Writable_declared
#define joedb_SQL_Dump_Writable_declared

#include "Dump_Writable.h"
#include "type_io.h"

#include <iostream>

namespace joedb
{
 class SQL_Dump_Writable: public Dump_Writable
 {
  private:
   void write_type(Type type)
   {
    switch(type.get_type_id())
    {
     case Type::Type_Id::null:
      out << "NULL";
     break;

     case Type::Type_Id::string:
      out << "TEXT";
     break;

     case Type::Type_Id::int32:
      out << "INTEGER";
     break;

     case Type::Type_Id::int64:
      out << "BIGINT";
     break;

     case Type::Type_Id::reference:
     {
      out << key_type << " REFERENCES ";
      out << '\"' << db.get_table_name(type.get_table_id()) << '\"';
     }
     break;

     case Type::Type_Id::boolean:
      out << "SMALLINT";
     break;

     case Type::Type_Id::float32:
      out << "REAL";
     break;

     case Type::Type_Id::float64:
      out << "REAL";
     break;

     case Type::Type_Id::int8:
      out << "SMALLINT";
     break;

     case Type::Type_Id::int16:
      out << "SMALLINT";
     break;
    }
   }

   std::string id_field_name = "\"__id\"";
   std::string key_type = "INTEGER";

  public:
   SQL_Dump_Writable(std::ostream &out): Dump_Writable(out) {}

   void create_table(const std::string &name) override
   {
    out << "CREATE TABLE \"" << name << "\"(" << id_field_name << ' ' << key_type << " PRIMARY KEY);\n";
    Schema_Writable::create_table(name);
   }

   void drop_table(Table_Id table_id) override
   {
    out << "DROP TABLE \"" << db.get_table_name(table_id) << "\";\n";
    Schema_Writable::drop_table(table_id);
   }

   void rename_table(Table_Id table_id,
                     const std::string &name) override
   {
    out << "ALTER TABLE \"" << db.get_table_name(table_id);
    out << "\" RENAME TO \"" << name << "\";\n";
    Schema_Writable::rename_table(table_id, name);
   }

   void add_field(Table_Id table_id,
                  const std::string &name,
                  Type type) override
   {
    out << "ALTER TABLE \"" << db.get_table_name(table_id);
    out << "\" ADD \"" << name << "\" ";
    write_type(type);
    out << ";\n";
    Schema_Writable::add_field(table_id, name, type);
   }

   void drop_field(Table_Id table_id, Field_Id field_id) override
   {
    out << "ALTER TABLE \"" << db.get_table_name(table_id);
    out << "\" DROP \"" << db.get_field_name(table_id, field_id) << "\";\n";
    Schema_Writable::drop_field(table_id, field_id);
   }

   void rename_field(Table_Id table_id,
                     Field_Id field_id,
                     const std::string &name) override
   {
    out << "ALTER TABLE \"" << db.get_table_name(table_id) << "\" RENAME COLUMN \"";
    out << db.get_field_name(table_id, field_id) << "\" TO \"" << name << "\";\n";
    Schema_Writable::rename_field(table_id, field_id, name);
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

   void insert_into(Table_Id table_id, Record_Id record_id) override
   {
    out << "INSERT INTO \"" << db.get_table_name(table_id);
    out << "\"(" << id_field_name << ") VALUES(" << record_id << ");\n";
   }

   void insert_vector(Table_Id table_id,
                      Record_Id record_id,
                      Record_Id size) override
   {
    for (Record_Id i = 0; i < size; i++)
     insert_into(table_id, record_id + i);
   }

   void delete_from(Table_Id table_id, Record_Id record_id) override
   {
    out << "DELETE FROM \"" << db.get_table_name(table_id);
    out << "\" WHERE " << id_field_name << " = " << record_id << ";\n";
   }

   #define TYPE_MACRO(type, return_type, type_id, R, W)\
   void update_##type_id(Table_Id table_id,\
                         Record_Id record_id,\
                         Field_Id field_id,\
                         return_type value) override\
   {\
    out << "UPDATE \"" << db.get_table_name(table_id);\
    out << "\" SET \"" << db.get_field_name(table_id, field_id) << "\" = ";\
    joedb::write_##type_id(out, value);\
    out << " WHERE " << id_field_name << " = " << record_id << ";\n";\
   }
   #define TYPE_MACRO_NO_STRING
   #include "TYPE_MACRO.h"
   #undef TYPE_MACRO_NO_STRING
   #undef TYPE_MACRO

   void update_string(Table_Id table_id,
                      Record_Id record_id,
                      Field_Id field_id,
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
