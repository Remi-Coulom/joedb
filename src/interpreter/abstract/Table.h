#ifndef joedb_Table_declared
#define joedb_Table_declared

#include <unordered_map>
#include <algorithm>
#include <limits>

#include "Field.h"
#include "Type.h"
#include "Value.h"
#include "index_types.h"

namespace joedb
{
 class Table
 {
  private:
   std::string name;

   record_id_t current_record_id;
   field_id_t current_field_id;

   std::unordered_map<field_id_t, Field> fields;
   std::unordered_map<record_id_t, std::vector<Value>> records;

  public:
   Table(const std::string &name):
    name(name),
    current_record_id(0),
    current_field_id(0)
   {}

   // TODO: use auto for all getters with C++ 2014

   const std::string &get_name() const {return name;}

   const std::unordered_map<field_id_t, Field> &get_fields() const
   {
    return fields;
   }

   const std::unordered_map<record_id_t, std::vector<Value>>
    &get_records() const
   {
    return records;
   }

   field_id_t find_field(const std::string &name)
   {
    for (const auto &field: fields)
     if (field.second.name == name)
      return field.first;
    return 0;
   }

   field_id_t add_field(const std::string &name, const Type &type)
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
     record.second.push_back(Value());

    return current_field_id;
   }

   bool drop_field(field_id_t field_id)
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
     record.second.erase(record.second.begin() + field_index);

    return true;
   }

   bool delete_record(record_id_t id)
   {
    return records.erase(id) > 0;
   }

   record_id_t insert_record()
   {
    records.insert(std::make_pair(++current_record_id,
                                  std::vector<Value>(fields.size())));
    return current_record_id;
   }

   bool update(record_id_t record_id, field_id_t field_id, const Value &value)
   {
    auto field = fields.find(field_id);
    if (field == fields.end())
     return false;

    auto record = records.find(record_id);
    if (record == records.end())
     return false;

    record->second[field->second.index] = value;
    return true;
   }
 };
}

#endif
