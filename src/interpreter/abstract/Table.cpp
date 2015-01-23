#include "Table.h"

#include <limits>

/////////////////////////////////////////////////////////////////////////////
field_id_t joedb::Table::find_field(const std::string &name) const
{
 for (auto &field: fields)
  if (field.second.name == name)
   return field.first;
 return 0;
}

/////////////////////////////////////////////////////////////////////////////
field_id_t joedb::Table::add_field(const std::string &name, const Type &type)
{
 if (find_field(name) ||
     current_field_id == std::numeric_limits<field_id_t>::max())
  return 0;

 Field field;
 field.index = field_id_t(fields.size());
 field.name = name;
 field.type = type;
 fields.insert(std::make_pair(++current_field_id, field));

 for (auto &record: records)
  record.values.push_back(Value(type.get_type_id()));

 return current_field_id;
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Table::drop_field(field_id_t field_id)
{
 auto it = fields.find(field_id);
 if (it == fields.end())
  return false;

 const field_id_t field_index = it->second.index;
 fields.erase(it);

 for (auto &field: fields)
  if (field.second.index > field_index)
   --field.second.index;

 for (auto &record: records)
  record.values.erase(record.values.begin() + field_index);

 return true;
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Table::delete_record(record_id_t record_id)
{
 if (record_id == 0 || record_id > records.size())
  return false;

 Record &record = records[record_id - 1];

 if (record.is_free)
  return false;

 record.is_free = true;
 for (auto v: record.values)
  v = Value();

 if (record.previous)
  records[record.previous - 1].next = record.next;
 else
  first_record = record.next;

 if (record.next)
  records[record.next - 1].previous = record.previous;

 record.previous = 0;
 record.next = first_free_record;
 if (record.next)
  records[record.next - 1].previous = record_id;
 first_free_record = record_id;

 return true;
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Table::insert_record(record_id_t record_id)
{
 while (record_id > records.size())
 {
  records.push_back(Record(fields.size()));
  Record &record = records.back();
  record.previous = 0;
  record.next = first_free_record;
  if (record.next)
   records[record.next - 1].previous = records.size();
  first_free_record = records.size();
 }
 Record &record = records[record_id - 1];

 if (!record.is_free)
  return false;

 record.is_free = false;

 if (record.previous)
  records[record.previous - 1].next = record.next;
 else
  first_free_record = record.next;

 if (record.next)
  records[record.next - 1].previous = record.previous;

 record.next = first_record;
 record.previous = 0;
 if (record.next)
  records[record.next - 1].previous = record_id;
 first_record = record_id;

 return true;
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Table::update(record_id_t record_id,
                          field_id_t field_id,
                          const Value &value)
{
 auto field = fields.find(field_id);
 if (field == fields.end())
  return false;

 if (record_id > records.size() || records[record_id - 1].is_free)
  return false;

 records[record_id - 1].values[field->second.index] = value;

 return true;
}
