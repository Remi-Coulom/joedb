#include "joedb/io/Interpreter_Dump_Writable.h"
#include "joedb/io/type_io.h"
#include "joedb/io/get_time_string.h"

#include <iostream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Dump_Writable::write_type(Type type)
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
 void Interpreter_Dump_Writable::create_table(const std::string &name)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "create_table " << name << '\n';
  schema.create_table(name);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Dump_Writable::drop_table(Table_Id table_id)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "drop_table " << schema.get_table_name(table_id) << '\n';
  schema.drop_table(table_id);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Dump_Writable::rename_table
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  const std::string &name
 )
 {
  out << "rename_table " << schema.get_table_name(table_id) << ' ' << name << '\n';
  schema.rename_table(table_id, name);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Dump_Writable::add_field
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  const std::string &name,
  Type type
 )
 {
  out << "add_field " << schema.get_table_name(table_id) << ' ' << name << ' ';
  write_type(type);
  out << '\n';
  schema.add_field(table_id, name, type);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Dump_Writable::drop_field
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Field_Id field_id
 )
 {
  out << "drop_field " << schema.get_table_name(table_id) << ' ';
  out << schema.get_field_name(table_id, field_id) << '\n';
  schema.drop_field(table_id, field_id);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Dump_Writable::rename_field
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Field_Id field_id,
  const std::string &name
 )
 {
  out << "rename_field " << schema.get_table_name(table_id) << ' ';
  out << schema.get_field_name(table_id, field_id) << ' ' << name << '\n';
  schema.rename_field(table_id, field_id, name);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Dump_Writable::custom(const std::string &name)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "custom " << name << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Dump_Writable::comment(const std::string &comment)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "comment ";
  write_string(out, comment);
  out << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Dump_Writable::timestamp(int64_t timestamp)
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "timestamp " << timestamp << ' ';
  out << get_time_string(timestamp) << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Dump_Writable::valid_data()
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "valid_data\n";
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Dump_Writable::insert_into
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
 void Interpreter_Dump_Writable::insert_vector
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Record_Id record_id,
  Record_Id size
 )
 {
  out << "insert_vector " << schema.get_table_name(table_id) << ' ';
  out << record_id << ' ' << size << '\n';
 }

 ////////////////////////////////////////////////////////////////////////////
 void Interpreter_Dump_Writable::delete_from
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
 void Interpreter_Dump_Writable::update_##type_id\
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
  joedb::write_##type_id(out, value);\
  out << '\n';\
 }\
 void Interpreter_Dump_Writable::update_vector_##type_id\
 (\
  Table_Id table_id,\
  Record_Id record_id,\
  Field_Id field_id,\
  Record_Id size,\
  const type *value\
 )\
 {\
  out << "update_vector " << schema.get_table_name(table_id) << ' ';\
  out << record_id << ' ';\
  out << schema.get_field_name(table_id, field_id) << ' ';\
  out << size;\
  for (Record_Id i = 0; i < size; i++)\
  {\
   out << ' ';\
   joedb::write_##type_id(out, value[i]);\
  }\
  out << '\n';\
 }
 #include "joedb/TYPE_MACRO.h"
}
