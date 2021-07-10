#include "joedb/interpreter/Database.h"
#include "joedb/Exception.h"
#include "joedb/is_identifier.h"

#include <sstream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Database::check_identifier
 ////////////////////////////////////////////////////////////////////////////
 (
  const char *message,
  const std::string &name
 ) const
 {
  if (!is_identifier(name))
  {
   std::ostringstream error_message;
   error_message << message << ": invalid identifier: " << name;
   throw Exception(error_message.str());
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 const std::map<Field_Id, std::string> &Database::get_fields
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id
 ) const
 {
  auto table_it = tables.find(table_id);
  if (table_it == tables.end())
   throw Exception("get_fields: invalid table_id");
  return table_it->second.field_names;
 }

 ////////////////////////////////////////////////////////////////////////////
 const Type &Database::get_field_type
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Field_Id field_id
 ) const
 {
  static Type null_type;
  auto table_it = tables.find(table_id);
  if (table_it == tables.end())
   return null_type;
  auto &fields = table_it->second.get_fields();
  auto field_it = fields.find(field_id);
  if (field_it == fields.end())
   return null_type;
  return field_it->second.get_type();
 }

 ////////////////////////////////////////////////////////////////////////////
 Record_Id Database::get_last_record_id(Table_Id table_id) const
 ////////////////////////////////////////////////////////////////////////////
 {
  auto table_it = tables.find(table_id);
  if (table_it == tables.end())
   return 0;
  return table_it->second.freedom.size();
 }

 ////////////////////////////////////////////////////////////////////////////
 bool Database::is_used
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Record_Id record_id
 ) const
 {
  auto table_it = tables.find(table_id);
  if (table_it == tables.end())
   return false;
  return table_it->second.freedom.is_used(record_id + 1);
 }

 ////////////////////////////////////////////////////////////////////////////
 const Compact_Freedom_Keeper &Database::get_freedom
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id
 ) const
 {
  auto table_it = tables.find(table_id);
  if (table_it == tables.end())
   throw Exception("bad table id");
  else
   return table_it->second.freedom;
 }

 #define TYPE_MACRO(type, return_type, type_id, R, W)\
 return_type Database::get_##type_id\
 (\
  Table_Id table_id,\
  Record_Id record_id,\
  Field_Id field_id\
 ) const\
 {\
  auto table_it = tables.find(table_id);\
  if (table_it == tables.end())\
   throw Exception("get: invalid table_id");\
  return table_it->second.get_##type_id(record_id, field_id);\
 }\
 const type &Database::get_##type_id##_storage\
 (\
  Table_Id table_id,\
  Record_Id record_id,\
  Field_Id field_id\
 ) const\
 {\
  auto table_it = tables.find(table_id);\
  if (table_it == tables.end())\
   throw Exception("get_storage: invalid table_id");\
  return *table_it->second.get_own_##type_id##_storage(record_id, field_id);\
 }
 #include "joedb/TYPE_MACRO.h"

 ////////////////////////////////////////////////////////////////////////////
 void Database::create_table(const std::string &name)
 ////////////////////////////////////////////////////////////////////////////
 {
  check_identifier("create_table", name);

  if (find_table(name))
   throw Exception("create_table: name already used: " + name);

  ++current_table_id;
  tables.insert(std::make_pair(current_table_id, Table()));
  table_names[current_table_id] = name;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Database::drop_table(Table_Id table_id)
 ////////////////////////////////////////////////////////////////////////////
 {
  auto it = tables.find(table_id);
  if (it == tables.end())
   throw Exception("drop_table: invalid table_id");
  table_names.erase(table_id);
  tables.erase(it);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Database::rename_table
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  const std::string &name
 )
 {
  check_identifier("rename_table", name);

  auto table_it = tables.find(table_id);
  if (table_it == tables.end())
   throw Exception("rename_table: invalid table_id");

  if (find_table(name) != 0)
   throw Exception("rename_table: name already used: " + name);

  table_names[table_id] = name;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Database::add_field
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  const std::string &name,
  Type type
 )
 {
  check_identifier("add_field", name);

  auto it = tables.find(table_id);
  if (it == tables.end())
   throw Exception("add_field: invalid table_id");

  it->second.add_field(name, type);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Database::drop_field(Table_Id table_id, Field_Id field_id)
 ////////////////////////////////////////////////////////////////////////////
 {
  auto it = tables.find(table_id);
  if (it == tables.end())
   throw Exception("drop_field: invalid table_id");

  it->second.drop_field(field_id);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Database::rename_field
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Field_Id field_id,
  const std::string &name
 )
 {
  check_identifier("rename_field", name);

  auto table_it = tables.find(table_id);
  if (table_it == tables.end())
   throw Exception("rename_field: invalid table_id");

  auto &field_names = table_it->second.field_names;
  auto field_it = field_names.find(field_id);
  if (field_it == field_names.end())
   throw Exception("rename_field: invalid field_id");

  if (table_it->second.find_field(name))
   throw Exception("rename_field: name already used: " + name);

  field_it->second = name;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Database::insert_into(Table_Id table_id, Record_Id record_id)
 ////////////////////////////////////////////////////////////////////////////
 {
  auto it = tables.find(table_id);
  if (it == tables.end())
   throw Exception("insert_into: invalid table_id");

  if (record_id <= 0 || (max_record_id && record_id > max_record_id))
   throw Exception("insert_into: too big");

  it->second.insert_record(record_id);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Database::insert_vector
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Record_Id record_id,
  Record_Id size
 )
 {
  auto it = tables.find(table_id);
  if (it == tables.end())
   throw Exception("insert_vector: invalid table_id");
  if (record_id <= 0 ||
      (max_record_id && (record_id > max_record_id || size > max_record_id)))
  {
   std::ostringstream error_message;
   error_message << "insert_vector: ";
   error_message << "record_id = " << record_id << "; ";
   error_message << "size = " << size << "; ";
   error_message << "max = " << max_record_id;
   throw Exception(error_message.str());
  }

  it->second.insert_vector(record_id, size);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Database::delete_from
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Record_Id record_id
 )
 {
  auto it = tables.find(table_id);
  if (it == tables.end())
   throw Exception("delete_from: invalid table_id");

  it->second.delete_record(record_id);
 }

 ////////////////////////////////////////////////////////////////////////////
 #define TYPE_MACRO(type, return_type, type_id, R, W)\
 void Database::update_##type_id\
 (\
  Table_Id table_id,\
  Record_Id record_id,\
  Field_Id field_id,\
  return_type value\
 )\
 {\
  auto it = tables.find(table_id);\
  if (it == tables.end())\
   throw Exception("update: invalid table_id");\
  it->second.update_##type_id(record_id, field_id, value);\
 }\
 \
 void Database::update_vector_##type_id\
 (\
  Table_Id table_id,\
  Record_Id record_id,\
  Field_Id field_id,\
  Record_Id size,\
  const type *value\
 )\
 {\
  auto it = tables.find(table_id);\
  if (it == tables.end())\
   throw Exception("update_vector: invalid table_id");\
  it->second.update_vector_##type_id(record_id, field_id, size, value);\
 }\
 \
 type *Database::get_own_##type_id##_storage\
 (\
  Table_Id table_id,\
  Record_Id record_id,\
  Field_Id field_id,\
  Record_Id &capacity\
 )\
 {\
  auto it = tables.find(table_id);\
  if (it == tables.end())\
   throw Exception("get_own_storage: invalid table_id");\
  capacity = it->second.get_storage_capacity();\
  return it->second.get_own_##type_id##_storage(record_id, field_id);\
 }
 #include "joedb/TYPE_MACRO.h"
}
