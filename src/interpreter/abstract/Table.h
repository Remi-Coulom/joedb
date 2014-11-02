#ifndef crazydb_Table_declared
#define crazydb_Table_declared

#include <unordered_map>
#include <vector>
#include <algorithm>

#include "Type.h"
#include "Value.h"
#include "index_types.h"

namespace crazydb
{
 class Table
 {
  private:
   record_id_t current_id;
   std::vector<std::string> field_names;
   std::vector<Type> field_types;
   std::unordered_map<record_id_t, std::vector<Value>> records;

  public:
   Table(): current_id(0) {}

   const std::vector<std::string> &get_field_names() const
   {
    return field_names;
   }
   const std::vector<Type> &get_field_types() const
   {
    return field_types;
   }
   const std::unordered_map<record_id_t, std::vector<Value>>
    &get_records() const
   {
    return records;
   }

   field_id_t get_field_id(const std::string &name)
   {
    auto it = find(field_names.begin(), field_names.end(), name);
    if (it == field_names.end())
     return 0;
    else
     return field_id_t(std::distance(field_names.begin(), it) + 1);
   }

   field_id_t add_field(const std::string &name, const Type &type)
   {
    if (get_field_id(name))
     return 0;

    field_names.push_back(name);
    field_types.push_back(type);

    for (auto record: records)
     record.second.push_back(Value());

    // TODO check limit when adding new
    // TODO check range when drop or update
    return field_id_t(field_names.size());
   }

   void drop_field(field_id_t field_id)
   {
    field_names.erase(field_names.begin() + field_id - 1);
    field_types.erase(field_types.begin() + field_id - 1);

    for (auto record: records)
     record.second.erase(record.second.begin() + field_id - 1);
   }

   bool delete_record(record_id_t id)
   {
    return records.erase(id) > 0;
   }

   record_id_t insert_record()
   {
    std::vector<Value> v;
    for (Type type: field_types)
     v.push_back(Value());
    records.insert(std::make_pair(++current_id, v));
    return current_id;
   }

   bool update(record_id_t record_id, field_id_t field_id, const Value &value)
   {
    auto record = records.find(record_id);
    if (record == records.end())
     return false;
    record->second[field_id - 1] = value;
    return true;
   }
 };
}

#endif
