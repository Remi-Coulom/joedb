#include "joedb/Readable.h"
#include "joedb/Exception.h"

namespace joedb
{
 const std::string Readable::default_table_name = "__unknown_table__";
 const std::string Readable::default_field_name = "__unknown_field__";
 Readable::~Readable() = default;

 ////////////////////////////////////////////////////////////////////////////
 Table_Id Readable::find_table(const std::string &name) const
 ////////////////////////////////////////////////////////////////////////////
 {
  for (const auto &table: get_tables())
   if (table.second == name)
    return table.first;
  return Table_Id(0);
 }

 ////////////////////////////////////////////////////////////////////////////
 Field_Id Readable::find_field(Table_Id table_id, const std::string &name) const
 ////////////////////////////////////////////////////////////////////////////
 {
  try
  {
   for (const auto &field: get_fields(table_id))
    if (field.second == name)
     return field.first;
  }
  catch (const Exception &)
  {
  }
  return Field_Id(0);
 }

 ////////////////////////////////////////////////////////////////////////////
 const std::string &Readable::get_table_name(Table_Id table_id) const
 ////////////////////////////////////////////////////////////////////////////
 {
  const std::map<Table_Id, std::string> &tables = get_tables();
  const auto it = tables.find(table_id);
  if (it == tables.end())
  {
   return default_table_name;
  }
  else
   return it->second;
 }

 ////////////////////////////////////////////////////////////////////////////
 const std::string &Readable::get_field_name
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Field_Id field_id
 ) const
 {
  try
  {
   const std::map<Field_Id, std::string> &fields = get_fields(table_id);
   const auto it = fields.find(field_id);
   if (it != fields.end())
    return it->second;
  }
  catch (const Exception &)
  {
  }

  return default_field_name;
 }
}
