#include "joedb/interpreted/Table.h"

#include <limits>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Field_Id Table::find_field(const std::string &name) const
 ////////////////////////////////////////////////////////////////////////////
 {
  for (const auto &[fid, fname]: field_names)
   if (fname == name)
    return fid;
  return Field_Id(0);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Table::add_field(const std::string &name, const Type &type)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (find_field(name) != Field_Id(0))
   throw Exception("add_field: name already used");
  if (current_field_id == Field_Id(std::numeric_limits<std::underlying_type<Field_Id>::type>::max()))
   throw Exception("add_field: reached maximum field count");

  ++current_field_id;
  fields.try_emplace(current_field_id, type, freedom.size());
  field_names.try_emplace(current_field_id, name);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Table::drop_field(Field_Id field_id)
 ////////////////////////////////////////////////////////////////////////////
 {
  const auto it = fields.find(field_id);
  if (it == fields.end())
   throw Exception("drop_field: invalid field_id");
  field_names.erase(field_id);
  fields.erase(it);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Table::delete_record(Record_Id record_id)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!freedom.is_used(record_id))
   throw Exception("delete_record: bad record_id");
  freedom.free(record_id);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Table::insert_record(Record_Id record_id)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (record_id >= freedom.get_size())
  {
   for (auto &field: fields)
    field.second.resize(to_underlying(record_id) + 1);
   while (freedom.get_size() <= record_id)
    freedom.push_back();
  }
  else if (!freedom.is_free(record_id))
   throw Exception("insert: record_id already in use");

  freedom.use(record_id);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Table::insert_vector(Record_Id record_id, size_t size)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (record_id == freedom.get_size())
  {
   freedom.resize(freedom.size() + size);
   for (auto &field: fields)
    field.second.resize(freedom.size());
   freedom.use_vector(record_id, Record_Id(size));
  }
  else
  {
   for (size_t i = 0; i < size; i++)
    insert_record(record_id + i);
  }
 }
}
