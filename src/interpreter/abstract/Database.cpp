#include "joedb/Database.h"

/////////////////////////////////////////////////////////////////////////////
table_id_t joedb::Database::create_table(const std::string &name)
/////////////////////////////////////////////////////////////////////////////
{
 if (find_table(name))
  return 0;

 tables.insert(std::make_pair(++current_table_id, Table(name)));
 listener->create_table(name);
 return current_table_id;
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Database::drop_table(table_id_t table_id)
/////////////////////////////////////////////////////////////////////////////
{
 if (tables.erase(table_id) > 0)
 {
  listener->drop_table(table_id);
  return true;
 }
 else
  return false;
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Database::rename_table
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 const std::string &name
)
{
 auto table_it = tables.find(table_id);
 if (table_it != tables.end() && find_table(name) == 0)
 {
  table_it->second.set_name(name);
  listener->rename_table(table_id, name);
  return true;
 }
 else
  return false;
}

/////////////////////////////////////////////////////////////////////////////
table_id_t joedb::Database::find_table(const std::string &name) const
/////////////////////////////////////////////////////////////////////////////
{
 for (const auto &table: tables)
  if (table.second.get_name() == name)
   return table.first;
 return 0;
}

/////////////////////////////////////////////////////////////////////////////
field_id_t joedb::Database::add_field
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 const std::string &name,
 Type type
)
{
 auto it = tables.find(table_id);
 if (it == tables.end())
  return 0;
 field_id_t field_id = it->second.add_field(name, type);
 if (field_id)
  listener->add_field(table_id, name, type);
 return field_id;
}

/////////////////////////////////////////////////////////////////////////////
field_id_t joedb::Database::find_field
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 const std::string &name
) const
{
 auto it = tables.find(table_id);
 if (it == tables.end())
  return 0;
 return it->second.find_field(name);
}

/////////////////////////////////////////////////////////////////////////////
joedb::Type::type_id_t joedb::Database::get_field_type
/////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////
{
 auto it = tables.find(table_id);
 if (it != tables.end() && it->second.drop_field(field_id))
 {
  listener->drop_field(table_id, field_id);
  return true;
 }
 return false;
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Database::rename_field
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 field_id_t field_id,
 const std::string &name
)
{
 auto table_it = tables.find(table_id);
 if (table_it != tables.end())
 {
  auto &fields = table_it->second.fields;
  auto field_it = fields.find(field_id);
  if (field_it != fields.end() && find_field(table_id, name) == 0)
  {
   field_it->second.set_name(name);
   listener->rename_field(table_id, field_id, name);
   return true;
  }
 }

 return false;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::custom(const std::string &name)
/////////////////////////////////////////////////////////////////////////////
{
 custom_names.push_back(name);
 listener->custom(name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::timestamp(int64_t timestamp) const
/////////////////////////////////////////////////////////////////////////////
{
 listener->timestamp(timestamp);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::comment(const std::string &comment) const
/////////////////////////////////////////////////////////////////////////////
{
 listener->comment(comment);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::valid_data() const
/////////////////////////////////////////////////////////////////////////////
{
 listener->valid_data();
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Database::insert_into(table_id_t table_id, record_id_t record_id)
/////////////////////////////////////////////////////////////////////////////
{
 auto it = tables.find(table_id);
 if (it != tables.end() && it->second.insert_record(record_id))
 {
  listener->insert(table_id, record_id);
  return true;
 }
 return false;
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Database::insert_vector
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id,
 record_id_t size
)
{
 auto it = tables.find(table_id);

 if (it != tables.end())
 {
  // TODO: optimize large vector insertion
  for (record_id_t i = 0; i < size; i++)
   it->second.insert_record(record_id + i);

  listener->insert_vector(table_id, record_id, size);
  return true;
 }

 return false;
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Database::delete_from(table_id_t table_id, record_id_t record_id)
/////////////////////////////////////////////////////////////////////////////
{
 auto it = tables.find(table_id);
 if (it != tables.end() && it->second.delete_record(record_id))
 {
  listener->delete_record(table_id, record_id);
  return true;
 }
 return false;
}

/////////////////////////////////////////////////////////////////////////////
#define TYPE_MACRO(type, return_type, type_id, R, W)\
bool joedb::Database::update_##type_id(table_id_t table_id,\
                                       record_id_t record_id,\
                                       field_id_t field_id,\
                                       return_type value)\
{\
 auto it = tables.find(table_id);\
 if (it != tables.end() &&\
     it->second.update_##type_id(record_id, field_id, value))\
 {\
  listener->update_##type_id(table_id, record_id, field_id, value);\
  return true;\
 }\
 return false;\
}\
bool joedb::Database::update_vector_##type_id(table_id_t table_id,\
                                              record_id_t record_id,\
                                              field_id_t field_id,\
                                              record_id_t size,\
                                              const type *value)\
{\
 auto it = tables.find(table_id);\
 if (it != tables.end() &&\
     it->second.update_vector_##type_id(record_id, field_id, size, value))\
 {\
  listener->update_vector_##type_id(table_id, record_id, field_id, size, value);\
  return true;\
 }\
 return false;\
}
#include "joedb/TYPE_MACRO.h"
#undef TYPE_MACRO
