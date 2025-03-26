#include "joedb/ui/Interpreter_Dump_Writable.h"
#include "joedb/ui/type_io.h"
#include "joedb/ui/get_time_string.h"

#include <iostream>

namespace joedb::ui
{
 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Writable::write_type(Type type)
 ////////////////////////////////////////////////////////////////////////////
 {
  switch(type.get_type_id())
  {
   case Type::Type_Id::null:
    out << "null";
   break;

   #define TYPE_MACRO(type, return_type, type_id, read, write)\
   case Type::Type_Id::type_id:\
    out << #type_id;\
   break;
   #define TYPE_MACRO_NO_REFERENCE
   #include "joedb/TYPE_MACRO.h"

   case Type::Type_Id::reference:
    out << "references " << schema.get_table_name(type.get_table_id());
   break;
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Writable::create_table(const std::string &name)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "create_table " << name << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Writable::drop_table(Table_Id table_id)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "drop_table " << schema.get_table_name(table_id) << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Writable::rename_table
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  const std::string &name
 )
 {
  out << "rename_table " << schema.get_table_name(table_id);
  out << ' ' << name << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Writable::add_field
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  const std::string &name,
  Type type
 )
 {
  out << "add_field " << schema.get_table_name(table_id);
  out << ' ' << name << ' ';
  write_type(type);
  out << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Writable::drop_field
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Field_Id field_id
 )
 {
  out << "drop_field " << schema.get_table_name(table_id) << ' ';
  out << schema.get_field_name(table_id, field_id) << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Writable::rename_field
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Field_Id field_id,
  const std::string &name
 )
 {
  out << "rename_field " << schema.get_table_name(table_id) << ' ';
  out << schema.get_field_name(table_id, field_id) << ' ' << name << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Writable::custom(const std::string &name)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "custom " << name << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Writable::comment(const std::string &comment)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "comment ";
  write_string(out, comment);
  out << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Writable::timestamp(int64_t timestamp)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "timestamp " << timestamp << ' ';
  out << get_time_string(timestamp) << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Writable::valid_data()
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "valid_data\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Writable::insert_into
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Record_Id record_id
 )
 {
  out << "insert_into " << schema.get_table_name(table_id) << ' ';
  out << record_id << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Writable::insert_vector
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Record_Id record_id,
  size_t size
 )
 {
  out << "insert_vector " << schema.get_table_name(table_id) << ' ';
  out << record_id << ' ' << size << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Writable::delete_from
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Record_Id record_id
 )
 {
  out << "delete_from " << schema.get_table_name(table_id) << ' ';
  out << record_id << '\n';
 }

 #define TYPE_MACRO(type, return_type, type_id, R, W)\
 void Interpreter_Writable::update_##type_id\
 (\
  Table_Id table_id,\
  Record_Id record_id,\
  Field_Id field_id,\
  return_type value\
 )\
 {\
  out << "update " << schema.get_table_name(table_id) << ' ';\
  out << record_id << ' ';\
  out << schema.get_field_name(table_id, field_id) << ' ';\
  write_##type_id(out, value);\
  out << '\n';\
 }\
 void Interpreter_Writable::update_vector_##type_id\
 (\
  Table_Id table_id,\
  Record_Id record_id,\
  Field_Id field_id,\
  size_t size,\
  const type *value\
 )\
 {\
  out << "update_vector " << schema.get_table_name(table_id) << ' ';\
  out << to_underlying(record_id) << ' ';\
  out << schema.get_field_name(table_id, field_id) << ' ';\
  out << size;\
  for (size_t i = 0; i < size; i++)\
  {\
   out << ' ';\
   write_##type_id(out, value[i]);\
  }\
  out << '\n';\
 }
 #include "joedb/TYPE_MACRO.h"

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Writable::on_blob(Blob blob, Blob_Reader &reader)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "# Blob: ";
  write_blob(out, blob);
  out << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 Blob Interpreter_Writable::write_blob_data(const std::string &data)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "write_blob ";
  write_string(out, data);
  out << '\n';

  return Blob();
 }

 ////////////////////////////////////////////////////////////////////////////
 Interpreter_Writable::~Interpreter_Writable() = default;
 ////////////////////////////////////////////////////////////////////////////
}
