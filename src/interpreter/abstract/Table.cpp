#include "Table.h"

#include <limits>

/////////////////////////////////////////////////////////////////////////////
field_id_t joedb::Table::find_field(const std::string &name) const
{
 for (auto &field: fields)
  if (field.second.get_name() == name)
   return field.first;
 return 0;
}

/////////////////////////////////////////////////////////////////////////////
field_id_t joedb::Table::add_field(const std::string &name, const Type &type)
{
 if (find_field(name) ||
     current_field_id == std::numeric_limits<field_id_t>::max())
  return 0;

 fields.insert(std::make_pair(++current_field_id,
                              Field(name, type, freedom.size())));

 return current_field_id;
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Table::drop_field(field_id_t field_id)
{
 auto it = fields.find(field_id);
 if (it == fields.end())
  return false;
 fields.erase(it);
 return true;
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Table::delete_record(record_id_t record_id)
{
 if (record_id == 0 ||
     record_id > freedom.size() ||
     freedom.is_free(record_id + 1))
  return false;
 freedom.free(record_id + 1);
 // TODO: reset default values
 return true;
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Table::insert_record(record_id_t record_id)
{
 if (record_id > freedom.size())
 {
  for (auto &field: fields)
   field.second.resize(record_id);
  while (freedom.size() < record_id)
   freedom.push_back();
 }
 else if (!freedom.is_free(record_id + 1))
  return false;

 freedom.use(record_id + 1);
 return true;
}
