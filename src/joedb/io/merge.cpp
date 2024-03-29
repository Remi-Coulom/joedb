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
  offset[table_id] = merged.get_last_record_id(table_id);
 }

 //
 // Second loop to copy data, with added offset
 //
 for (auto table: merged.get_tables())
 {
  const Table_Id table_id = table.first;
  const Record_Id last_record_id = db.get_last_record_id(table_id);
  const Compact_Freedom_Keeper &freedom_keeper = db.get_freedom(table_id);

  if (last_record_id == Record_Id(0))
  {
   // do nothing, table is empty
  }
  else if (freedom_keeper.is_compact())
  {
   merged.insert_vector(table_id, offset[table_id] + 1, size_t(last_record_id));

   for (const auto &field: db.get_fields(table_id))
   {
    const Field_Id field_id = field.first;
    const Type &type = db.get_field_type(table_id, field_id);
    size_t capacity;

    switch (type.get_type_id())
    {
     case Type::Type_Id::null:
     break;

     #define TYPE_MACRO(type, return_type, type_id, R, W)\
     case Type::Type_Id::type_id:\
     {\
      merged.update_vector_##type_id\
      (\
       table_id,\
       offset[table_id] + 1,\
       field_id,\
       size_t(last_record_id),\
       db.get_own_##type_id##_const_storage(table_id, Record_Id(1), field_id, capacity)\
      );\
     }\
     break;
     #include "joedb/TYPE_MACRO.h"
    }

    if (type.get_type_id() == Type::Type_Id::reference)
    {
     const size_t reference_offset = size_t(offset[type.get_table_id()]);

     Record_Id *reference = merged.get_own_reference_storage
     (
      table_id,
      offset[table_id] + 1,
      field_id,
      capacity
     );

     for (size_t i = 0; i < size_t(last_record_id); i++)
      if (reference[i] != Record_Id(0))
       reference[i] = reference[i] + reference_offset;
    }
   }
  }
  else
  {
   for (Record_Id record_id = Record_Id(1); size_t(record_id) <= size_t(last_record_id); ++record_id)
   {
    if (freedom_keeper.is_used(size_t(record_id) + 1))
    {
     const Record_Id merged_record_id = offset[table_id] + size_t(record_id);
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
        if (referenced != Record_Id(0))
         referenced = referenced + size_t(offset[type.get_table_id()]);
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
      }
     }
    }
   }
  }
 }
}
