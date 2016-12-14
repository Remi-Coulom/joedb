#include "Safe_Writeable.h"

// TODO: should also check that identifier are valid C++ identifiers
// TODO: don't allow "__" inside identifiers
// TODO: more detailed error for updates

/////////////////////////////////////////////////////////////////////////////
bool joedb::Safe_Writeable::is_existing_table_id(table_id_t table_id) const
/////////////////////////////////////////////////////////////////////////////
{
 const auto &tables = db.get_tables();
 return tables.find(table_id) != tables.end();
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Safe_Writeable::is_update_ok
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id,
 field_id_t field_id,
 record_id_t size,
 Type::type_id_t type_id
) const
{
 const auto &tables = db.get_tables();
 auto table_it = tables.find(table_id);

 if (table_it != tables.end())
 {
  const auto &fields = table_it->second.get_fields();
  auto field_it = fields.find(field_id);
  if (field_it != fields.end() &&
      field_it->second.get_type().get_type_id() == type_id)
  {
   const Freedom_Keeper<> &freedom = table_it->second.get_freedom();

   for (record_id_t i = 0; i < size; i++)
    if (!freedom.is_used(record_id + i + 1))
     return false;

   return true;
  }
 }

 return false;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Writeable::create_table(const std::string &name)
/////////////////////////////////////////////////////////////////////////////
{
 if (db.find_table(name))
  throw std::runtime_error("create_table: name already used");

 db_listener.create_table(name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Writeable::drop_table(table_id_t table_id)
/////////////////////////////////////////////////////////////////////////////
{
 if (!is_existing_table_id(table_id))
  throw std::runtime_error("drop_table: invalid table_id");

 db_listener.drop_table(table_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Writeable::rename_table
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 const std::string &name
)
{
 if (!is_existing_table_id(table_id))
  throw std::runtime_error("rename_table: invalid table_id");
 if (db.find_table(name))
  throw std::runtime_error("rename_table: name already used");

 db_listener.rename_table(table_id, name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Writeable::add_field
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 const std::string &name,
 Type type
)
{
 const auto &tables = db.get_tables();
 const auto table_it = tables.find(table_id);

 if (table_it == tables.end())
  throw std::runtime_error("add_field: invalid table_id");

 if (table_it->second.find_field(name))
  throw std::runtime_error("add_field: name already used");

 db_listener.add_field(table_id, name, type);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Writeable::drop_field
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 field_id_t field_id
)
{
 const auto &tables = db.get_tables();
 const auto table_it = tables.find(table_id);
 if (table_it == tables.end())
  throw std::runtime_error("drop_field: invalid table_id");

 const auto &fields = table_it->second.get_fields();
 auto field_it = fields.find(field_id);
 if (field_it == fields.end())
  throw std::runtime_error("drop_field: invalid field_id");

 db_listener.drop_field(table_id, field_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Writeable::rename_field
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 field_id_t field_id,
 const std::string &name
)
{
 const auto &tables = db.get_tables();
 const auto table_it = tables.find(table_id);
 if (table_it == tables.end())
  throw std::runtime_error("rename_field: invalid table_id");

 const auto &fields = table_it->second.get_fields();
 auto field_it = fields.find(field_id);
 if (field_it == fields.end())
  throw std::runtime_error("rename_field: invalid field_id");

 if (table_it->second.find_field(name))
  throw std::runtime_error("rename_field: name already used");

 db_listener.rename_field(table_id, field_id, name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Writeable::custom(const std::string &name)
/////////////////////////////////////////////////////////////////////////////
{
 db_listener.custom(name);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Writeable::comment(const std::string &comment)
/////////////////////////////////////////////////////////////////////////////
{
 db_listener.comment(comment);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Writeable::timestamp(int64_t timestamp)
/////////////////////////////////////////////////////////////////////////////
{
 db_listener.timestamp(timestamp);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Writeable::valid_data()
/////////////////////////////////////////////////////////////////////////////
{
 db_listener.valid_data();
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Writeable::insert
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id
)
{
 if (!is_existing_table_id(table_id))
  throw std::runtime_error("insert: invalid table_id");
 if (record_id <= 0 || (max_record_id && record_id > max_record_id))
  throw std::runtime_error("insert: bad record_id");
 // TODO: check that it does not already exist?
 db_listener.insert(table_id, record_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Writeable::insert_vector
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id,
 record_id_t size
)
{
 if (!is_existing_table_id(table_id))
  throw std::runtime_error("insert_vector: invalid table_id");
 if (record_id <= 0 ||
     size <= 0 ||
     (max_record_id && record_id > max_record_id) ||
     (max_record_id && size > max_record_id))
  throw std::runtime_error("insert_vector: bad record_id or size");
 db_listener.insert_vector(table_id, record_id, size);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Safe_Writeable::delete_record
/////////////////////////////////////////////////////////////////////////////
(
 table_id_t table_id,
 record_id_t record_id
)
{
 const auto &tables = db.get_tables();
 const auto table_it = tables.find(table_id);
 if (table_it == tables.end())
  throw std::runtime_error("delete: invalid table_id");
 // TODO: check that it already exists

 db_listener.delete_record(table_id, record_id);
}

#define TYPE_MACRO(type, return_type, type_id, R, W)\
void joedb::Safe_Writeable::update_##type_id(table_id_t table_id,\
                                            record_id_t record_id,\
                                            field_id_t field_id,\
                                            return_type value)\
{\
 if (is_update_ok(table_id,\
                  record_id,\
                  field_id,\
                  1,\
                  Type::type_id_t::type_id))\
  db_listener.update_##type_id(table_id, record_id, field_id, value);\
 else\
  throw std::runtime_error("Wrong update");\
}\
void joedb::Safe_Writeable::update_vector_##type_id(table_id_t table_id,\
                                                   record_id_t record_id,\
                                                   field_id_t field_id,\
                                                   record_id_t size,\
                                                   const type *value)\
{\
 if (is_update_ok(table_id,\
                  record_id,\
                  field_id,\
                  size,\
                  Type::type_id_t::type_id))\
  db_listener.update_vector_##type_id(table_id,\
                                      record_id,\
                                      field_id,\
                                      size,\
                                      value);\
 else\
  throw std::runtime_error("Wrong update");\
}
#include "TYPE_MACRO.h"
#undef TYPE_MACRO
