#include "Safe_Listener.h"

/////////////////////////////////////////////////////////////////////////////
bool joedb::Safe_Listener::is_existing_table_id(table_id_t table_id) const
/////////////////////////////////////////////////////////////////////////////
{
 const auto &tables = db.get_tables();
 return tables.find(table_id) != tables.end();
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_create_table(const std::string &name)
/////////////////////////////////////////////////////////////////////////////
{
 if (db.find_table(name))
  throw std::runtime_error("after_create_table: " + name + " exists.");
 listener.after_create_table(name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_drop_table(table_id_t table_id)
/////////////////////////////////////////////////////////////////////////////
{
 if (!is_existing_table_id(table_id))
  throw std::runtime_error("after_drop_table: invalid table_id");
 listener.after_drop_table(table_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_rename_table
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 const std::string &name
)
{
 if (!is_existing_table_id(table_id))
  throw std::runtime_error("after_rename_table: invalid table_id");
 listener.after_rename_table(table_id, name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_add_field
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 const std::string &name,
 Type type
)
{
 listener.after_add_field(table_id, name, type);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_drop_field
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 field_id_t field_id
)
{
 listener.after_drop_field(table_id, field_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_rename_field
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 field_id_t field_id,
 const std::string &name
)
{
 listener.after_rename_field(table_id, field_id, name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_custom(const std::string &name)
/////////////////////////////////////////////////////////////////////////////
{
 listener.after_custom(name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_comment(const std::string &comment)
/////////////////////////////////////////////////////////////////////////////
{
 listener.after_comment(comment);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_timestamp(int64_t timestamp)
/////////////////////////////////////////////////////////////////////////////
{
 listener.after_timestamp(timestamp);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_valid_data()
/////////////////////////////////////////////////////////////////////////////
{
 listener.after_valid_data();
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_insert
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id
)
{
 if (!is_existing_table_id(table_id))
  throw std::runtime_error("after_insert: invalid table_id");
 if (record_id <= 0 || (safe_insert && record_id > checkpoint_position))
  throw std::runtime_error("after_insert: bad record_id");
 listener.after_insert(table_id, record_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_insert_vector
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id,
 record_id_t size
)
{
 if (!is_existing_table_id(table_id))
  throw std::runtime_error("after_insert_vector: invalid table_id");
 if (record_id <= 0 ||
     size <= 0 ||
     (safe_insert && record_id > checkpoint_position) ||
     (safe_insert && size > checkpoint_position))
  throw std::runtime_error("after_insert_vector: bad record_id or size");
 listener.after_insert_vector(table_id, record_id, size);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Listener::after_delete
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id
)
{
 listener.after_delete(table_id, record_id);
}

#define TYPE_MACRO(type, return_type, type_id, R, W)\
void joedb::Safe_Listener::after_update_##type_id(table_id_t table_id,\
                                                  record_id_t record_id,\
                                                  field_id_t field_id,\
                                                  return_type value)\
{\
 const auto &tables = db.get_tables();\
 auto table_it = tables.find(table_id);\
 if (table_it != tables.end())\
 {\
  const auto &fields = table_it->second.get_fields();\
  auto field_it = fields.find(field_id);\
  if (field_it != fields.end())\
  {\
   if (field_it->second.get_type().get_type_id() == Type::type_id_t::type_id)\
   {\
    listener.after_delete(table_id, record_id);\
    return;\
   }\
  }\
 }\
 throw std::runtime_error("Wrong update");\
}
#include "TYPE_MACRO.h"
#undef TYPE_MACRO
