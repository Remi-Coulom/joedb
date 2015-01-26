#include "Database.h"

/////////////////////////////////////////////////////////////////////////////
table_id_t joedb::Database::create_table(const std::string &name)
{
 if (find_table(name))
  return 0;

 tables.insert(std::make_pair(++current_table_id, Table(name)));
 listener->after_create_table(name);
 return current_table_id;
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Database::drop_table(table_id_t table_id)
{
 if (tables.erase(table_id) > 0)
 {
  listener->after_drop_table(table_id);
  return true;
 }
 else
  return false;
}

/////////////////////////////////////////////////////////////////////////////
table_id_t joedb::Database::find_table(const std::string &name) const
{
 for (const auto &table: tables)
  if (table.second.get_name() == name)
   return table.first;
 return 0;
}

/////////////////////////////////////////////////////////////////////////////
field_id_t joedb::Database::add_field(table_id_t table_id,
                                      const std::string &name,
                                      Type type)
{
 auto it = tables.find(table_id);
 if (it == tables.end())
  return 0;
 field_id_t field_id = it->second.add_field(name, type);
 if (field_id)
  listener->after_add_field(table_id, name, type);
 return field_id;
}

/////////////////////////////////////////////////////////////////////////////
field_id_t joedb::Database::find_field(table_id_t table_id,
                                       const std::string &name) const
{
 auto it = tables.find(table_id);
 if (it == tables.end())
  return 0;
 return it->second.find_field(name);
}

/////////////////////////////////////////////////////////////////////////////
joedb::Type::type_id_t joedb::Database::get_field_type
(
 table_id_t table_id,
 field_id_t field_id
) const
{
 auto table_it = tables.find(table_id);
 if (table_it == tables.end())
  return Type::type_id_t::null;
 auto &fields = table_it->second.get_fields();
 auto field_it = fields.find(field_id);
 if (field_it == fields.end())
  return Type::type_id_t::null;
 return field_it->second.get_type().get_type_id();
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Database::drop_field(table_id_t table_id, field_id_t field_id)
{
 auto it = tables.find(table_id);
 if (it != tables.end() && it->second.drop_field(field_id))
 {
  listener->after_drop_field(table_id, field_id);
  return true;
 }
 return false;
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Database::insert_into(table_id_t table_id, record_id_t record_id)
{
 auto it = tables.find(table_id);
 if (it != tables.end() && it->second.insert_record(record_id))
 {
  listener->after_insert(table_id, record_id);
  return true;
 }
 return false;
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Database::delete_from(table_id_t table_id, record_id_t record_id)
{
 auto it = tables.find(table_id);
 if (it != tables.end() && it->second.delete_record(record_id))
 {
  listener->after_delete(table_id, record_id);
  return true;
 }
 return false;
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Database::update_string(table_id_t table_id,
                                    record_id_t record_id,
                                    field_id_t field_id,
                                    const std::string &value)
{
 auto it = tables.find(table_id);
 if (it != tables.end() && it->second.update_string(record_id, field_id, value))
 {
  listener->after_update_string(table_id, record_id, field_id, value);
  return true;
 }
 return false;
}
