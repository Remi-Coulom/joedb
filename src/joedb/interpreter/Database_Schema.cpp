#include "joedb/interpreter/Database_Schema.h"
#include "joedb/Exception.h"
#include "joedb/is_identifier.h"

namespace joedb::interpreter
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
   throw Exception(std::string(message) + ": invalid identifier: " + name);
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 const Table& Database_Schema::get_table
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id
 ) const
 {
  return (const_cast<Database_Schema *>(this))->get_table(table_id);
 }

 ////////////////////////////////////////////////////////////////////////////
 Table& Database_Schema::get_table
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id
 )
 {
  const auto it = tables.find(table_id);

  if (it == tables.end())
  {
   throw Exception
   (
    "get_table: invalid table_id: " + std::to_string(to_underlying(table_id))
   );
  }

  return it->second;
 }

 ////////////////////////////////////////////////////////////////////////////
 const std::map<Field_Id, std::string> &Database_Schema::get_fields
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id
 ) const
 {
  return get_table(table_id).field_names;
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
 const Compact_Freedom_Keeper &Database_Schema::get_freedom
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id
 ) const
 {
  return get_table(table_id).freedom;
 }

 #define TYPE_MACRO(type, return_type, type_id, R, W)\
 const type &Database_Schema::get_##type_id\
 (\
  Table_Id table_id,\
  Record_Id record_id,\
  Field_Id field_id\
 ) const\
 {\
  return *get_table(table_id).get_own_##type_id##_storage(record_id, field_id);\
 }
 #include "joedb/TYPE_MACRO.h"

 ////////////////////////////////////////////////////////////////////////////
 void Database_Schema::create_table(const std::string &name)
 ////////////////////////////////////////////////////////////////////////////
 {
  check_identifier("create_table", name);

  if (find_table(name) != Table_Id(0))
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

  if (find_table(name) != Table_Id(0))
   throw Exception("rename_table: name already used: " + name);

  get_table(table_id); // make sure the table exists

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

  get_table(table_id).add_field(name, type);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Database_Schema::drop_field(Table_Id table_id, Field_Id field_id)
 ////////////////////////////////////////////////////////////////////////////
 {
  get_table(table_id).drop_field(field_id);
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

  Table &table = get_table(table_id);

  auto &field_names = table.field_names;
  const auto field_it = field_names.find(field_id);
  if (field_it == field_names.end())
   throw Exception("rename_field: invalid field_id");

  if (table.find_field(name) != Field_Id(0))
   throw Exception("rename_field: name already used: " + name);

  field_it->second = name;
 }

 ////////////////////////////////////////////////////////////////////////////
 Database_Schema::~Database_Schema() = default;
 ////////////////////////////////////////////////////////////////////////////
}
