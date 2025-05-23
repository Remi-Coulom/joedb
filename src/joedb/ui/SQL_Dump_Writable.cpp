#include "joedb/ui/SQL_Dump_Writable.h"
#include "joedb/ui/type_io.h"
#include "joedb/ui/get_time_string.h"
#include "joedb/journal/Buffered_File.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void SQL_Writable::write_type(Type type)
 ////////////////////////////////////////////////////////////////////////////
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
    out << '\"' << schema.get_table_name(type.get_table_id()) << '\"';
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

   case Type::Type_Id::blob:
    out << "BLOB";
   break;
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void SQL_Writable::start_writing(int64_t position)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "BEGIN TRANSACTION; -- checkpoint = " << position << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void SQL_Writable::end_writing(int64_t position)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "COMMIT; --checkpoint = " << position << '\n';
  out.flush();
 }

 ////////////////////////////////////////////////////////////////////////////
 void SQL_Writable::create_table(const std::string &name)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "CREATE TABLE \"" << name << "\"(" << id_field_name <<
         ' ' << key_type << " PRIMARY KEY);\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 void SQL_Writable::drop_table(Table_Id table_id)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "DROP TABLE \"" << schema.get_table_name(table_id) << "\";\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 void SQL_Writable::rename_table
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  const std::string &name
 )
 {
  out << "ALTER TABLE \"" << schema.get_table_name(table_id);
  out << "\" RENAME TO \"" << name << "\";\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 void SQL_Writable::add_field
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  const std::string &name,
  Type type
 )
 {
  out << "ALTER TABLE \"" << schema.get_table_name(table_id);
  out << "\" ADD \"" << name << "\" ";
  write_type(type);
  out << ";\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 void SQL_Writable::drop_field(Table_Id table_id, Field_Id field_id)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "ALTER TABLE \"" << schema.get_table_name(table_id);
  out << "\" DROP COLUMN \"" << schema.get_field_name(table_id, field_id) << "\";\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 void SQL_Writable::rename_field
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Field_Id field_id,
  const std::string &name
 )
 {
  out << "ALTER TABLE \"" << schema.get_table_name(table_id) << "\" RENAME COLUMN \"";
  out << schema.get_field_name(table_id, field_id) << "\" TO \"" << name << "\";\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 void SQL_Writable::custom(const std::string &name)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "-- custom: " << name << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void SQL_Writable::comment(const std::string &comment)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "-- " << comment << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void SQL_Writable::timestamp(int64_t timestamp)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "-- " << get_time_string(timestamp) << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void SQL_Writable::valid_data()
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "-- valid data\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 void SQL_Writable::insert_into(Table_Id table_id, Record_Id record_id)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "INSERT INTO \"" << schema.get_table_name(table_id);
  out << "\"(" << id_field_name << ") VALUES(" << record_id << ");\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 void SQL_Writable::delete_from(Table_Id table_id, Record_Id record_id)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "DELETE FROM \"" << schema.get_table_name(table_id) << '"';
  write_where(record_id);
 }

 ////////////////////////////////////////////////////////////////////////////
 void SQL_Writable::write_update(Table_Id table_id, Field_Id field_id)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "UPDATE \"" << schema.get_table_name(table_id);
  out << "\" SET \"" << schema.get_field_name(table_id, field_id) << "\" = ";
 }

 ////////////////////////////////////////////////////////////////////////////
 void SQL_Writable::write_where(Record_Id record_id)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << " WHERE " << id_field_name << " = " << record_id << ";\n";
 }

 #define TYPE_MACRO(type, return_type, type_id, R, W)\
 void SQL_Writable::update_##type_id\
 (\
  Table_Id table_id,\
  Record_Id record_id,\
  Field_Id field_id,\
  return_type value\
 )\
 {\
  write_update(table_id, field_id);\
  write_##type_id(out, value);\
  write_where(record_id);\
 }
 #define TYPE_MACRO_NO_STRING
 #define TYPE_MACRO_NO_BLOB
 #define TYPE_MACRO_NO_REFERENCE
 #define TYPE_MACRO_NO_BOOL
 #include "joedb/TYPE_MACRO.h"

 ////////////////////////////////////////////////////////////////////////////
 void SQL_Writable::update_boolean
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Record_Id record_id,
  Field_Id field_id,
  const bool value)
 {
  write_update(table_id, field_id);
  out << value;
  write_where(record_id);
 }

 ////////////////////////////////////////////////////////////////////////////
 void SQL_Writable::update_string
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Record_Id record_id,
  Field_Id field_id,
  const std::string &value)
 {
  write_update(table_id, field_id);
  write_sql_string(out, value);
  write_where(record_id);
 }

 ////////////////////////////////////////////////////////////////////////////
 void SQL_Writable::update_blob
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Record_Id record_id,
  Field_Id field_id,
  Blob value)
 {
  if (!blob_reader)
   out << "-- ";

  write_update(table_id, field_id);

  if (blob_reader)
   write_sql_string(out, blob_reader->read_blob(value));
  else
   out << "\"BLOB\"";

  write_where(record_id);
 }

 ////////////////////////////////////////////////////////////////////////////
 void SQL_Writable::update_reference
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Record_Id record_id,
  Field_Id field_id,
  Record_Id value
 )
 {
  write_update(table_id, field_id);

  if (value.is_null())
   out << "NULL";
  else
   out << value;

  write_where(record_id);
 }

 ////////////////////////////////////////////////////////////////////////////
 SQL_Writable::~SQL_Writable() = default;
 ////////////////////////////////////////////////////////////////////////////
}
