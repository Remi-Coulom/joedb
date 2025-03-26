#include "joedb/interpreted/Database.h"
#include "joedb/Exception.h"

#include <sstream>

namespace joedb::interpreted
{
 ////////////////////////////////////////////////////////////////////////////
 void Database::insert_into(Table_Id table_id, Record_Id record_id)
 ////////////////////////////////////////////////////////////////////////////
 {
  if
  (
   to_underlying(record_id) <= 0 ||
   (max_record_id && to_underlying(record_id) > max_record_id)
  )
  {
   throw Exception("insert_into: too big");
  }

  get_table(table_id).insert_record(record_id);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Database::insert_vector
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Record_Id record_id,
  size_t size
 )
 {
  if
  (
   to_underlying(record_id) <= 0 ||
   (
    max_record_id &&
    (
     to_underlying(record_id) > max_record_id ||
     size > max_record_id
    )
   )
  )
  {
   std::ostringstream error_message;
   error_message << "insert_vector: record_id = ";
   error_message << to_underlying(record_id);
   error_message << "; size = " << size;
   error_message << "; max = " << max_record_id;
   throw Exception(error_message.str());
  }

  get_table(table_id).insert_vector(record_id, size);
 }

 ////////////////////////////////////////////////////////////////////////////
 void Database::delete_from
 ////////////////////////////////////////////////////////////////////////////
 (
  Table_Id table_id,
  Record_Id record_id
 )
 {
  get_table(table_id).delete_record(record_id);
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
  get_table(table_id).update_##type_id(record_id, field_id, value);\
 }\
 void Database::update_vector_##type_id\
 (\
  Table_Id table_id,\
  Record_Id record_id,\
  Field_Id field_id,\
  size_t size,\
  const type *value\
 )\
 {\
  get_table(table_id).update_vector_##type_id(record_id, field_id, size, value);\
 }\
 \
 type *Database::get_own_##type_id##_storage\
 (\
  Table_Id table_id,\
  Record_Id record_id,\
  Field_Id field_id,\
  size_t &capacity\
 )\
 {\
  Table &table = get_table(table_id);\
  capacity = table.get_storage_capacity();\
  return table.get_own_##type_id##_storage(record_id, field_id);\
 }
 #include "joedb/TYPE_MACRO.h"

 ////////////////////////////////////////////////////////////////////////////
 Database::~Database() = default;
 ////////////////////////////////////////////////////////////////////////////
}
