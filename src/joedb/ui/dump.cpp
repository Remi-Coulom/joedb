#include "joedb/ui/dump.h"
#include "joedb/interpreted/Database.h"
#include "joedb/Writable.h"

#include "joedb/Selective_Writable.h"
#include "joedb/Multiplexer.h"
#include "joedb/journal/Readonly_Journal.h"

namespace joedb::ui
{
 /////////////////////////////////////////////////////////////////////////////
 void dump(const Readable &db, Writable &writable, bool schema_only)
 /////////////////////////////////////////////////////////////////////////////
 {
  //
  // Dump tables
  //
  std::map<Table_Id, Table_Id> table_map;
  {
   Table_Id mapped_tid = Table_Id(0);
   for (const auto &[tid, tname]: db.get_tables())
   {
    ++mapped_tid;
    table_map[tid] = mapped_tid;
    writable.create_table(tname);
   }
  }

  //
  // Dump fields
  //
  std::map<Table_Id, std::map<Field_Id, Field_Id>> field_maps;
  {
   for (const auto &[tid, tname]: db.get_tables())
   {
    Field_Id mapped_fid = Field_Id(0);
    for (const auto &[fid, fname]: db.get_fields(tid))
    {
     ++mapped_fid;
     Type type = db.get_field_type(tid, fid);
     if (type.get_type_id() == Type::Type_Id::reference)
      type = Type::reference(table_map[type.get_table_id()]);
     field_maps[tid][fid] = mapped_fid;

     writable.add_field(table_map[tid], fname, type);
    }
   }
  }

  //
  // Dump records
  //
  if (schema_only)
   return;

  for (const auto &[tid, tname]: db.get_tables())
  {
   const size_t last_record_id = size_t(db.get_last_record_id(tid));

   for (Record_Id record_id = Record_Id(1); size_t(record_id) <= last_record_id;)
   {
    while
    (
     size_t(record_id) <= last_record_id &&
     !db.is_used(tid, record_id)
    )
    {
     ++record_id;
    }

    size_t size = 0;

    while
    (
     size_t(record_id) + size <= last_record_id &&
     db.is_used(tid, record_id + size)
    )
    {
     size++;
    }

    if (size)
    {
     writable.insert_vector(table_map[tid], record_id, size);
     record_id = record_id + size;
    }
   }

   for (const auto &[fid, fname]: db.get_fields(tid))
   {
    for (Record_Id record_id = Record_Id(1); size_t(record_id) <= last_record_id; ++record_id)
    {
     if (db.is_used(tid, record_id))
     {
      Table_Id mapped_tid = table_map[tid];
      Field_Id mapped_fid = field_maps[tid][fid];

      switch (db.get_field_type(tid, fid).get_type_id())
      {
       case Type::Type_Id::null:
       break;

       #define TYPE_MACRO(type, return_type, type_id, R, W)\
       case Type::Type_Id::type_id:\
        writable.update_##type_id\
        (\
         mapped_tid, record_id, mapped_fid, db.get_##type_id(tid, record_id, fid)\
        );\
       break;
       #include "joedb/TYPE_MACRO.h"
      }
     }
    }
   }
  }
 }

 /////////////////////////////////////////////////////////////////////////////
 void dump_data(const Readable &db, Writable &writable)
 /////////////////////////////////////////////////////////////////////////////
 {
  for (const auto &[tid, tname]: db.get_tables())
  {
   const size_t last_record_id = size_t(db.get_last_record_id(tid));

   Record_Id record_id = Record_Id(1);

   const Compact_Freedom_Keeper &freedom_keeper = db.get_freedom(tid);

   while (size_t(record_id) <= last_record_id)
   {
    while
    (
     size_t(record_id) <= last_record_id &&
     !freedom_keeper.is_used(size_t(record_id) + 1)
    )
    {
     ++record_id;
    }

    size_t size = 0;

    while
    (
     size_t(record_id) + size <= last_record_id &&
     freedom_keeper.is_used(size_t(record_id) + size + 1)
    )
    {
     size++;
    }

    if (size > 1)
    {
     writable.insert_vector(tid, record_id, size);

     for (const auto &[fid, fname]: db.get_fields(tid))
     {
      switch(db.get_field_type(tid, fid).get_type_id())
      {
       case Type::Type_Id::null:
       break;

       #define TYPE_MACRO(type, return_type, type_id, R, W)\
       case Type::Type_Id::type_id:\
       {\
        writable.update_vector_##type_id\
        (\
         tid,\
         record_id,\
         fid,\
         size,\
         &db.get_##type_id\
         (\
          tid,\
          record_id,\
          fid\
         )\
        );\
       }\
       break;
       #include "joedb/TYPE_MACRO.h"
      }
     }
    }
    else if (size == 1)
    {
     writable.insert_into(tid, record_id);

     for (const auto &[fid, fname]: db.get_fields(tid))
     {
      switch(db.get_field_type(tid, fid).get_type_id())
      {
       case Type::Type_Id::null:
       break;

       #define TYPE_MACRO(type, return_type, type_id, R, W)\
       case Type::Type_Id::type_id:\
       {\
        writable.update_##type_id\
        (\
         tid,\
         record_id,\
         fid,\
         db.get_##type_id(tid, record_id, fid)\
        );\
       }\
       break;
       #include "joedb/TYPE_MACRO.h"
      }
     }
    }

    record_id = record_id + size;
   }
  }
  writable.default_checkpoint();
 }

 /////////////////////////////////////////////////////////////////////////////
 void pack(Readonly_Journal &input_journal, Writable &writable)
 /////////////////////////////////////////////////////////////////////////////
 {
  interpreted::Database db;

  {
   Selective_Writable schema_filter(writable, Selective_Writable::Mode::schema);
   Multiplexer multiplexer{db, schema_filter};

   input_journal.replay_log(multiplexer);

   if (schema_filter.has_blobs())
    throw Exception("can't pack blobs");
  }

  dump_data(db, writable);
 }
}
