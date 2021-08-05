#include "joedb/interpreter/Database.h"
#include "joedb/Exception.h"

#include <sstream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Database::insert_into(Table_Id table_id, Record_Id record_id)
 ////////////////////////////////////////////////////////////////////////////
 {
  auto it = tables.find(table_id);
  if (it == tables.end())
   throw Exception("insert_into: invalid table_id");

  if (record_id <= 0 || (max_record_id && record_id > max_record_id))
   throw Exception("insert_into: too big");

  it->second.insert_record(record_id);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Database::insert_vector
 ////////////////////////////////////////////////////////////////////////////
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
      (max_record_id && (record_id > max_record_id || size > max_record_id)))
  {
   std::ostringstream error_message;
   error_message << "insert_vector: ";
   error_message << "record_id = " << record_id << "; ";
   error_message << "size = " << size << "; ";
   error_message << "max = " << max_record_id;
   throw Exception(error_message.str());
  }

  it->second.insert_vector(record_id, size);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Database::delete_from
 ////////////////////////////////////////////////////////////////////////////
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

 ////////////////////////////////////////////////////////////////////////////
 #define TYPE_MACRO(type, return_type, type_id, R, W)\
 void Database::update_##type_id\
 (\
  Table_Id table_id,\
  Record_Id record_id,\
  Field_Id field_id,\
  return_type value\
 )\
 {\
  auto it = tables.find(table_id);\
  if (it == tables.end())\
   throw Exception("update: invalid table_id");\
  it->second.update_##type_id(record_id, field_id, value);\
 }\
 \
 void Database::update_vector_##type_id\
 (\
  Table_Id table_id,\
  Record_Id record_id,\
  Field_Id field_id,\
  Record_Id size,\
  const type *value\
 )\
 {\
  auto it = tables.find(table_id);\
  if (it == tables.end())\
   throw Exception("update_vector: invalid table_id");\
  it->second.update_vector_##type_id(record_id, field_id, size, value);\
 }\
 \
 type *Database::get_own_##type_id##_storage\
 (\
  Table_Id table_id,\
  Record_Id record_id,\
  Field_Id field_id,\
  Record_Id &capacity\
 )\
 {\
  auto it = tables.find(table_id);\
  if (it == tables.end())\
   throw Exception("get_own_storage: invalid table_id");\
  capacity = it->second.get_storage_capacity();\
  return it->second.get_own_##type_id##_storage(record_id, field_id);\
 }
 #include "joedb/TYPE_MACRO.h"
}
