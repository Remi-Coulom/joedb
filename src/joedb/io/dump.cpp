#include "joedb/io/dump.h"
#include "joedb/interpreter/Database.h"
#include "joedb/Writable.h"

#include "joedb/Selective_Writable.h"
#include "joedb/Multiplexer.h"
#include "joedb/journal/Readonly_Journal.h"

/////////////////////////////////////////////////////////////////////////////
void joedb::dump(const Readable &db, Writable &writable, bool schema_only)
/////////////////////////////////////////////////////////////////////////////
{
 //
 // Dump tables
 //
 std::map<Table_Id, Table_Id> table_map;
 {
  Table_Id table_id = 0;
  for (auto table: db.get_tables())
  {
   ++table_id;
   table_map[table.first] = table_id;

   writable.create_table(table.second);
  }
 }

 //
 // Dump fields
 //
 std::map<Table_Id, std::map<Field_Id, Field_Id>> field_maps;
 {
  Table_Id table_id = 0;
  for (auto table: db.get_tables())
  {
   ++table_id;
   Field_Id field_id = 0;
   for (const auto &field: db.get_fields(table.first))
   {
    ++field_id;
    Type type = db.get_field_type(table.first, field.first);
    if (type.get_type_id() == Type::Type_Id::reference)
     type = Type::reference(table_map[type.get_table_id()]);
    field_maps[table.first][field.first] = field_id;

    writable.add_field(table_map[table.first], field.second, type);
   }
  }
 }

 //
 // Dump records
 //
 if (schema_only)
  return;

 for (auto table: db.get_tables())
 {
  const Table_Id table_id = table.first;
  const Record_Id last_record_id = db.get_last_record_id(table_id);

  Record_Id record_id = 1;

  while (record_id <= last_record_id)
  {
   while (record_id <= last_record_id &&
          !db.is_used(table_id, record_id))
    record_id++;

   Record_Id size = 0;

   while (record_id + size <= last_record_id &&
          db.is_used(table_id, record_id + size))
    size++;

   if (size)
   {
    writable.insert_vector(table_map[table_id], record_id, size);
    record_id += size;
   }
  }

  for (const auto &field: db.get_fields(table_id))
  {
   const Field_Id field_id = field.first;

   for (Record_Id record_id = 1; record_id <= last_record_id; record_id++)
    if (db.is_used(table_id, record_id))
    {
     switch (db.get_field_type(table_id, field_id).get_type_id())
     {
      case Type::Type_Id::null:
      break;

      #define TYPE_MACRO(type, return_type, type_id, R, W)\
      case Type::Type_Id::type_id:\
       writable.update_##type_id(table_map[table_id], record_id, field_maps[table_id][field_id], db.get_##type_id(table_id, record_id, field_id));\
      break;
      #include "joedb/TYPE_MACRO.h"
     }
    }
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
void joedb::dump_data(const Readable &db, Writable &writable)
/////////////////////////////////////////////////////////////////////////////
{
 for (auto table: db.get_tables())
 {
  const Table_Id table_id = table.first;
  const Record_Id last_record_id = db.get_last_record_id(table_id);

  Record_Id record_id = 1;

  const Compact_Freedom_Keeper &freedom_keeper = db.get_freedom(table_id);

  while (record_id <= last_record_id)
  {
   while
   (
    record_id <= last_record_id &&
    !freedom_keeper.is_used(record_id + 1)
   )
    record_id++;

   Record_Id size = 0;

   while
   (
    record_id + size <= last_record_id &&
    freedom_keeper.is_used(record_id + size + 1)
   )
    size++;

   if (size)
   {
    writable.insert_vector(table_id, record_id, size);

    for (const auto &field: db.get_fields(table_id))
    {
     const Field_Id field_id = field.first;

     switch(db.get_field_type(table_id, field_id).get_type_id())
     {
      case Type::Type_Id::null:
      break;

      #define TYPE_MACRO(type, return_type, type_id, R, W)\
      case Type::Type_Id::type_id:\
      {\
       writable.update_vector_##type_id\
       (\
        table_id,\
        record_id,\
        field_id,\
        size,\
        &db.get_##type_id##_storage\
        (\
         table_id,\
         record_id,\
         field_id\
        )\
       );\
      }\
      break;
      #include "joedb/TYPE_MACRO.h"
     }
    }

    record_id += size;
   }
  }
 }
 writable.checkpoint(Commit_Level::no_commit);
}

/////////////////////////////////////////////////////////////////////////////
void joedb::pack(Readonly_Journal &input_journal, Writable &writable)
/////////////////////////////////////////////////////////////////////////////
{
 Database db;

 Selective_Writable schema_filter(writable, Selective_Writable::Mode::schema);
 Multiplexer multiplexer;
 multiplexer.add_writable(db);
 multiplexer.add_writable(schema_filter);

 input_journal.replay_log(multiplexer);

 dump_data(db, writable);
}
