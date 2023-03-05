#include "joedb/interpreter/Database_Schema.h"
#include "joedb/Exception.h"
#include "joedb/is_identifier.h"

#include <sstream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Database_Schema::check_identifier
 ////////////////////////////////////////////////////////////////////////////
 (
  const char *message,
  const std::string &name
 )
 {
  if (!is_identifier(name))
  {
   std::ostringstream error_message;
   error_message << message << ": invalid identifier: " << name;
   throw Exception(error_message.str());
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 const std::map<Field_Id, std::string> &Database_Schema::get_fields
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id
 ) const
 {
  const auto table_it = tables.find(table_id);
  if (table_it == tables.end())
   throw Exception("get_fields: invalid table_id");
  return table_it->second.field_names;
 }

 ////////////////////////////////////////////////////////////////////////////
 const Type &Database_Schema::get_field_type
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Field_Id field_id
 ) const
 {
  static Type null_type;
  const auto table_it = tables.find(table_id);
  if (table_it == tables.end())
   return null_type;
  const auto &fields = table_it->second.get_fields();
  const auto field_it = fields.find(field_id);
  if (field_it == fields.end())
   return null_type;
  return field_it->second.get_type();
 }

 ////////////////////////////////////////////////////////////////////////////
 Record_Id Database_Schema::get_last_record_id(Table_Id table_id) const
 ////////////////////////////////////////////////////////////////////////////
 {
  const auto table_it = tables.find(table_id);
  if (table_it == tables.end())
   return 0;
  return table_it->second.freedom.size();
 }

 ////////////////////////////////////////////////////////////////////////////
 bool Database_Schema::is_used
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Record_Id record_id
 ) const
 {
  const auto table_it = tables.find(table_id);
  if (table_it == tables.end())
   return false;
  return table_it->second.freedom.is_used(record_id + 1);
 }

 ////////////////////////////////////////////////////////////////////////////
 const Compact_Freedom_Keeper &Database_Schema::get_freedom
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id
 ) const
 {
  const auto table_it = tables.find(table_id);
  if (table_it == tables.end())
   throw Exception("bad table id");
  else
   return table_it->second.freedom;
 }

 #define TYPE_MACRO(type, return_type, type_id, R, W)\
 return_type Database_Schema::get_##type_id\
 (\
  Table_Id table_id,\
  Record_Id record_id,\
  Field_Id field_id\
 ) const\
 {\
  const auto table_it = tables.find(table_id);\
  if (table_it == tables.end())\
   throw Exception("get: invalid table_id");\
  return table_it->second.get_##type_id(record_id, field_id);\
 }\
 const type &Database_Schema::get_##type_id##_storage\
 (\
  Table_Id table_id,\
  Record_Id record_id,\
  Field_Id field_id\
 ) const\
 {\
  const auto table_it = tables.find(table_id);\
  if (table_it == tables.end())\
   throw Exception("get_storage: invalid table_id");\
  return *table_it->second.get_own_##type_id##_storage(record_id, field_id);\
 }
 #include "joedb/TYPE_MACRO.h"

 ////////////////////////////////////////////////////////////////////////////
 void Database_Schema::create_table(const std::string &name)
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
 void Database_Schema::drop_table(Table_Id table_id)
 ////////////////////////////////////////////////////////////////////////////
 {
  const auto it = tables.find(table_id);
  if (it == tables.end())
   throw Exception("drop_table: invalid table_id");
  table_names.erase(table_id);
  tables.erase(it);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Database_Schema::rename_table
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  const std::string &name
 )
 {
  check_identifier("rename_table", name);

  const auto table_it = tables.find(table_id);
  if (table_it == tables.end())
   throw Exception("rename_table: invalid table_id");

  if (find_table(name) != 0)
   throw Exception("rename_table: name already used: " + name);

  table_names[table_id] = name;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Database_Schema::add_field
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  const std::string &name,
  Type type
 )
 {
  check_identifier("add_field", name);

  const auto it = tables.find(table_id);
  if (it == tables.end())
   throw Exception("add_field: invalid table_id");

  it->second.add_field(name, type);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Database_Schema::drop_field(Table_Id table_id, Field_Id field_id)
 ////////////////////////////////////////////////////////////////////////////
 {
  const auto it = tables.find(table_id);
  if (it == tables.end())
   throw Exception("drop_field: invalid table_id");

  it->second.drop_field(field_id);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Database_Schema::rename_field
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Field_Id field_id,
  const std::string &name
 )
 {
  check_identifier("rename_field", name);

  const auto table_it = tables.find(table_id);
  if (table_it == tables.end())
   throw Exception("rename_field: invalid table_id");

  auto &field_names = table_it->second.field_names;
  const auto field_it = field_names.find(field_id);
  if (field_it == field_names.end())
   throw Exception("rename_field: invalid field_id");

  if (table_it->second.find_field(name))
   throw Exception("rename_field: name already used: " + name);

  field_it->second = name;
 }

 ////////////////////////////////////////////////////////////////////////////
 Database_Schema::~Database_Schema() = default;
 ////////////////////////////////////////////////////////////////////////////
}
