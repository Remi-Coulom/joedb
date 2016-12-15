#include "joedb/Database.h"
#include "is_identifier.h"

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
void joedb::Database::create_table(const std::string &name)
/////////////////////////////////////////////////////////////////////////////
{
 if (!is_identifier(name))
  throw std::runtime_error("create_table: invalid identifier");
 if (find_table(name))
  throw std::runtime_error("create_table: name already used");

 tables.insert(std::make_pair(++current_table_id, Table(name)));
 writeable->create_table(name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::drop_table(table_id_t table_id)
/////////////////////////////////////////////////////////////////////////////
{
 if (tables.erase(table_id) == 0)
  throw std::runtime_error("drop_table: invalid table_id");

 writeable->drop_table(table_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::rename_table
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 const std::string &name
)
{
 if (!is_identifier(name))
  throw std::runtime_error("rename_table: invalid identifier");

 auto table_it = tables.find(table_id);
 if (table_it == tables.end())
  throw std::runtime_error("rename_table: invalid table_id");

 if (find_table(name) == 0)
 {
  table_it->second.set_name(name);
  writeable->rename_table(table_id, name);
 }
 else
  throw std::runtime_error("rename_table: name already used");
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::add_field
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 const std::string &name,
 Type type
)
{
 if (!is_identifier(name))
  throw std::runtime_error("add_field: invalid identifier");

 auto it = tables.find(table_id);
 if (it == tables.end())
  throw std::runtime_error("add_field: invalid table_id");

 it->second.add_field(name, type);
 writeable->add_field(table_id, name, type);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::drop_field(table_id_t table_id, field_id_t field_id)
/////////////////////////////////////////////////////////////////////////////
{
 auto it = tables.find(table_id);
 if (it == tables.end())
  throw std::runtime_error("drop_field: invalid table_id");

 it->second.drop_field(field_id);
 writeable->drop_field(table_id, field_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::rename_field
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 field_id_t field_id,
 const std::string &name
)
{
 if (!is_identifier(name))
  throw std::runtime_error("rename_field: invalid identifier");

 auto table_it = tables.find(table_id);
 if (table_it == tables.end())
  throw std::runtime_error("rename_field: invalid table_id");

 auto &fields = table_it->second.fields;
 auto field_it = fields.find(field_id);
 if (field_it == fields.end())
  throw std::runtime_error("rename_field: invalid field_id");

 if (table_it->second.find_field(name))
  throw std::runtime_error("rename_field: name already used");

 field_it->second.set_name(name);
 writeable->rename_field(table_id, field_id, name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::custom(const std::string &name)
/////////////////////////////////////////////////////////////////////////////
{
 if (!is_identifier(name))
  throw std::runtime_error("custom: invalid identifier");
 custom_names.push_back(name);
 writeable->custom(name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::timestamp(int64_t timestamp)
/////////////////////////////////////////////////////////////////////////////
{
 writeable->timestamp(timestamp);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::comment(const std::string &comment)
/////////////////////////////////////////////////////////////////////////////
{
 writeable->comment(comment);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::valid_data()
/////////////////////////////////////////////////////////////////////////////
{
 writeable->valid_data();
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::insert(table_id_t table_id, record_id_t record_id)
/////////////////////////////////////////////////////////////////////////////
{
 auto it = tables.find(table_id);
 if (it == tables.end())
  throw std::runtime_error("insert: invalid table_id");

 if (record_id <= 0 || (max_record_id && record_id > max_record_id))
  throw std::runtime_error("insert: too big");

 it->second.insert_record(record_id);
 writeable->insert(table_id, record_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::insert_vector
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id,
 record_id_t size
)
{
 auto it = tables.find(table_id);
 if (it == tables.end())
  throw std::runtime_error("insert_vector: invalid table_id");
 if (record_id <= 0 ||
     size <= 0 ||
     (max_record_id && (record_id > max_record_id || size > max_record_id)))
  throw std::runtime_error("insert_vector: too big");

 // TODO: optimize large vector insertion

 for (record_id_t i = 0; i < size; i++)
  it->second.insert_record(record_id + i);

 writeable->insert_vector(table_id, record_id, size);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::delete_record
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id
)
{
 auto it = tables.find(table_id);
 if (it == tables.end())
  throw std::runtime_error("delete: invalid table_id");

 it->second.delete_record(record_id);
 writeable->delete_record(table_id, record_id);
}

/////////////////////////////////////////////////////////////////////////////
#define TYPE_MACRO(type, return_type, type_id, R, W)\
void joedb::Database::update_##type_id(table_id_t table_id,\
                                       record_id_t record_id,\
                                       field_id_t field_id,\
                                       return_type value)\
{\
 auto it = tables.find(table_id);\
 if (it == tables.end())\
  throw std::runtime_error("update: invalid table_id");\
 it->second.update_##type_id(record_id, field_id, value);\
 writeable->update_##type_id(table_id, record_id, field_id, value);\
}\
void joedb::Database::update_vector_##type_id(table_id_t table_id,\
                                              record_id_t record_id,\
                                              field_id_t field_id,\
                                              record_id_t size,\
                                              const type *value)\
{\
 auto it = tables.find(table_id);\
 if (it == tables.end())\
  throw std::runtime_error("update_vector: invalid table_id");\
 it->second.update_vector_##type_id(record_id, field_id, size, value);\
 writeable->update_vector_##type_id(table_id, record_id, field_id, size, value);\
}
#include "joedb/TYPE_MACRO.h"
#undef TYPE_MACRO
