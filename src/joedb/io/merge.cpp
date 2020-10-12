#include "joedb/io/merge.h"
#include "joedb/interpreter/Database.h"

/////////////////////////////////////////////////////////////////////////////
void joedb::merge(Database &merged, const Database &db)
/////////////////////////////////////////////////////////////////////////////
{
 std::map<Table_Id, Record_Id> offset;

 //
 // First loop over tables to fill the offset map
 //
 for (auto table: merged.get_tables())
 {
  const Table_Id table_id = table.first;
  const Record_Id last_record_id = merged.get_last_record_id(table_id);
  offset[table_id] = last_record_id;
 }

 //
 // Second loop to copy data, with added offset
 //
 for (auto table: merged.get_tables())
 {
  const Table_Id table_id = table.first;
  const Record_Id last_record_id = db.get_last_record_id(table_id);
  for (Record_Id record_id = 1; record_id <= last_record_id; record_id++)
   if (db.is_used(table_id, record_id))
   {
    const Record_Id merged_record_id = record_id + offset[table_id];
    merged.insert_into(table_id, merged_record_id);

    for (const auto &field: db.get_fields(table_id))
    {
     const Field_Id field_id = field.first;
     const Type &type = db.get_field_type(table_id, field_id);

     switch (type.get_type_id())
     {
      case Type::Type_Id::null:
      break;

      case Type::Type_Id::reference:
      {
       Record_Id referenced = db.get_reference(table_id, record_id, field_id);
       if (referenced > 0)
        referenced += offset[type.get_table_id()];
       merged.update_reference
       (
        table_id,
        merged_record_id,
        field_id,
        referenced
       );
      }
      break;

      #define TYPE_MACRO(type, return_type, type_id, R, W)\
      case Type::Type_Id::type_id:\
      {\
       merged.update_##type_id\
       (\
        table_id,\
        merged_record_id,\
        field_id,\
        db.get_##type_id(table_id, record_id, field_id)\
       );\
      }\
      break;
      #define TYPE_MACRO_NO_REFERENCE
      #include "joedb/TYPE_MACRO.h"
      #undef TYPE_MACRO
     }
    }
   }
 }
}
