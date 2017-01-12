#include "Database.h"
#include "Exception.h"
#include "is_identifier.h"

/////////////////////////////////////////////////////////////////////////////
const std::map<Field_Id, std::string> &joedb::Database::get_fields
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id
) const
{
 auto table_it = tables.find(table_id);
 if (table_it == tables.end())
  throw Exception("get_fields: invalid table_id");
 return table_it->second.field_names;
}

/////////////////////////////////////////////////////////////////////////////
const joedb::Type &joedb::Database::get_field_type
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id,
 Field_Id field_id
) const
{
 static Type null_type;
 auto table_it = tables.find(table_id);
 if (table_it == tables.end())
  return null_type;
 auto &fields = table_it->second.get_fields();
 auto field_it = fields.find(field_id);
 if (field_it == fields.end())
  return null_type;
 return field_it->second.get_type();
}

/////////////////////////////////////////////////////////////////////////////
Record_Id joedb::Database::get_last_record_id(Table_Id table_id) const
/////////////////////////////////////////////////////////////////////////////
{
 auto table_it = tables.find(table_id);
 if (table_it == tables.end())
  return 0;
 return table_it->second.freedom.size();
}

/////////////////////////////////////////////////////////////////////////////
bool joedb::Database::is_used
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id,
 Record_Id record_id
) const
{
 auto table_it = tables.find(table_id);
 if (table_it == tables.end())
  return false;
 return table_it->second.freedom.is_used(record_id + 1);
}

#define TYPE_MACRO(type, return_type, type_id, R, W)\
return_type joedb::Database::get_##type_id(Table_Id table_id,\
                                           Record_Id record_id,\
                                           Field_Id field_id) const\
{\
 auto table_it = tables.find(table_id);\
 if (table_it == tables.end())\
  throw Exception("get: invalid table_id");\
 return table_it->second.get_##type_id(record_id, field_id);\
}
#include "TYPE_MACRO.h"
#undef TYPE_MACRO

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::create_table(const std::string &name)
/////////////////////////////////////////////////////////////////////////////
{
 if (!is_identifier(name))
  throw Exception("create_table: invalid identifier");
 if (find_table(name))
  throw Exception("create_table: name already used");

 ++current_table_id;
 tables.insert(std::make_pair(current_table_id, Table()));
 table_names[current_table_id] = name;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::drop_table(Table_Id table_id)
/////////////////////////////////////////////////////////////////////////////
{
 auto it = tables.find(table_id);
 if (it == tables.end())
  throw Exception("drop_table: invalid table_id");
 table_names.erase(table_id);
 tables.erase(it);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::rename_table
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id,
 const std::string &name
)
{
 if (!is_identifier(name))
  throw Exception("rename_table: invalid identifier");

 auto table_it = tables.find(table_id);
 if (table_it == tables.end())
  throw Exception("rename_table: invalid table_id");

 if (find_table(name) != 0)
  throw Exception("rename_table: name already used");

 table_names[table_id] = name;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::add_field
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id,
 const std::string &name,
 Type type
)
{
 if (!is_identifier(name))
  throw Exception("add_field: invalid identifier");

 auto it = tables.find(table_id);
 if (it == tables.end())
  throw Exception("add_field: invalid table_id");

 it->second.add_field(name, type);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::drop_field(Table_Id table_id, Field_Id field_id)
/////////////////////////////////////////////////////////////////////////////
{
 auto it = tables.find(table_id);
 if (it == tables.end())
  throw Exception("drop_field: invalid table_id");

 it->second.drop_field(field_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::rename_field
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id,
 Field_Id field_id,
 const std::string &name
)
{
 if (!is_identifier(name))
  throw Exception("rename_field: invalid identifier");

 auto table_it = tables.find(table_id);
 if (table_it == tables.end())
  throw Exception("rename_field: invalid table_id");

 auto &field_names = table_it->second.field_names;
 auto field_it = field_names.find(field_id);
 if (field_it == field_names.end())
  throw Exception("rename_field: invalid field_id");

 if (table_it->second.find_field(name))
  throw Exception("rename_field: name already used");

 field_it->second = name;
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::insert_into(Table_Id table_id, Record_Id record_id)
/////////////////////////////////////////////////////////////////////////////
{
 auto it = tables.find(table_id);
 if (it == tables.end())
  throw Exception("insert_into: invalid table_id");

 if (record_id <= 0 || (max_record_id && record_id > max_record_id))
  throw Exception("insert_into: too big");

 it->second.insert_record(record_id);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::insert_vector
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id,
 Record_Id record_id,
 Record_Id size
)
{
 auto it = tables.find(table_id);
 if (it == tables.end())
  throw Exception("insert_vector: invalid table_id");
 if (record_id <= 0 ||
     size <= 0 ||
     (max_record_id && (record_id > max_record_id || size > max_record_id)))
  throw Exception("insert_vector: too big");

 for (Record_Id i = 0; i < size; i++)
  it->second.insert_record(record_id + i);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::Database::delete_from
/////////////////////////////////////////////////////////////////////////////
(
 Table_Id table_id,
 Record_Id record_id
)
{
 auto it = tables.find(table_id);
 if (it == tables.end())
  throw Exception("delete_from: invalid table_id");

 it->second.delete_record(record_id);
}

/////////////////////////////////////////////////////////////////////////////
#define TYPE_MACRO(type, return_type, type_id, R, W)\
void joedb::Database::update_##type_id(Table_Id table_id,\
                                       Record_Id record_id,\
                                       Field_Id field_id,\
                                       return_type value)\
{\
 auto it = tables.find(table_id);\
 if (it == tables.end())\
  throw Exception("update: invalid table_id");\
 it->second.update_##type_id(record_id, field_id, value);\
}\
void joedb::Database::update_vector_##type_id(Table_Id table_id,\
                                              Record_Id record_id,\
                                              Field_Id field_id,\
                                              Record_Id size,\
                                              const type *value)\
{\
 auto it = tables.find(table_id);\
 if (it == tables.end())\
  throw Exception("update_vector: invalid table_id");\
 it->second.update_vector_##type_id(record_id, field_id, size, value);\
}
#include "TYPE_MACRO.h"
#undef TYPE_MACRO
