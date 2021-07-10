#include "joedb/interpreter/Table.h"

#include <limits>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Field_Id Table::find_field(const std::string &name) const
 ////////////////////////////////////////////////////////////////////////////
 {
  for (auto &field: field_names)
   if (field.second == name)
    return field.first;
  return 0;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Table::add_field(const std::string &name, const Type &type)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (find_field(name))
   throw Exception("add_field: name already used");
  if (current_field_id == std::numeric_limits<Field_Id>::max())
   throw Exception("add_field: reached maximum field count");

  ++current_field_id;
  fields.insert(std::make_pair(current_field_id, Field(type, freedom.size())));
  field_names[current_field_id] = name;
 }

 ////////////////////////////////////////////////////////////////////////////
 void Table::drop_field(Field_Id field_id)
 ////////////////////////////////////////////////////////////////////////////
 {
  auto it = fields.find(field_id);
  if (it == fields.end())
   throw Exception("drop_field: invalid field_id");
  field_names.erase(field_id);
  fields.erase(it);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Table::delete_record(Record_Id record_id)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!freedom.is_used(record_id + 1))
   throw Exception("delete_record: bad record_id");
  freedom.free(record_id + 1);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Table::insert_record(Record_Id record_id)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (record_id > freedom.size())
  {
   for (auto &field: fields)
    field.second.resize(record_id);
   while (freedom.size() < record_id)
    freedom.push_back();
  }
  else if (!freedom.is_free(record_id + 1))
   throw Exception("insert: record_id already in use");

  freedom.use(record_id + 1);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Table::insert_vector(Record_Id record_id, Record_Id size)
 ////////////////////////////////////////////////////////////////////////////
 {
  if (freedom.is_compact() && record_id == freedom.size() + 1)
  {
   for (auto &field: fields)
    field.second.resize(record_id + size - 1);

   freedom.append_vector(size);
  }
  else
  {
   for (Record_Id i = 0; i < size; i++)
    insert_record(record_id + i);
  }
 }
}
