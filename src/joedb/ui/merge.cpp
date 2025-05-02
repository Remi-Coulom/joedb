#include "joedb/ui/merge.h"
#include "joedb/interpreted/Database.h"

/////////////////////////////////////////////////////////////////////////////
void joedb::merge(Database &merged, const Database &db)
/////////////////////////////////////////////////////////////////////////////
{
 std::map<Table_Id, Record_Id> offset;

 //
 // First loop over tables to fill the offset map
 //
 for (const auto &[tid, tname]: merged.get_tables())
  offset[tid] = merged.get_last_record_id(tid);

 //
 // Second loop to copy data, with added offset
 //
 for (const auto &[tid, tname]: merged.get_tables())
 {
  const Record_Id last_record_id = db.get_last_record_id(tid);
  const Compact_Freedom_Keeper &freedom_keeper = db.get_freedom(tid);

  if (last_record_id == Record_Id(0))
  {
   // do nothing, table is empty
  }
  else if (freedom_keeper.is_compact())
  {
   merged.insert_vector(tid, offset[tid] + 1, size_t(last_record_id));

   for (const auto &[fid, fname]: db.get_fields(tid))
   {
    const Type &type = db.get_field_type(tid, fid);
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
       tid,\
       offset[tid] + 1,\
       fid,\
       size_t(last_record_id),\
       db.get_own_##type_id##_const_storage(tid, Record_Id(1), fid, capacity)\
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
      tid,
      offset[tid] + 1,
      fid,
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
    if (freedom_keeper.is_used(to_underlying(record_id) + 1))
    {
     const Record_Id merged_record_id = offset[tid] + size_t(record_id);
     merged.insert_into(tid, merged_record_id);

     for (const auto &[fid, fname]: db.get_fields(tid))
     {
      const Type &type = db.get_field_type(tid, fid);

      switch (type.get_type_id())
      {
       case Type::Type_Id::null:
       break;

       case Type::Type_Id::reference:
       {
        Record_Id referenced = db.get_reference(tid, record_id, fid);
        if (referenced != Record_Id(0))
         referenced = referenced + size_t(offset[type.get_table_id()]);
        merged.update_reference
        (
         tid,
         merged_record_id,
         fid,
         referenced
        );
       }
       break;

       #define TYPE_MACRO(type, return_type, type_id, R, W)\
       case Type::Type_Id::type_id:\
       {\
        merged.update_##type_id\
        (\
         tid,\
         merged_record_id,\
         fid,\
         db.get_##type_id(tid, record_id, fid)\
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
