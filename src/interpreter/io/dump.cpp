#include "dump.h"
#include "joedb/Database.h"
#include "joedb/Writeable.h"

/////////////////////////////////////////////////////////////////////////////
void joedb::dump(const Readable &db, Writeable &writeable)
/////////////////////////////////////////////////////////////////////////////
{
 //
 // Dump tables
 //
 std::map<table_id_t, table_id_t> table_map;
 {
  table_id_t table_id = 0;
  for (auto table: db.get_tables())
  {
   ++table_id;
   table_map[table.first] = table_id;

   writeable.create_table(table.second);
  }
 }

 //
 // Dump fields
 //
 std::map<table_id_t, std::map<field_id_t, field_id_t>> field_maps;
 {
  table_id_t table_id = 0;
  for (auto table: db.get_tables())
  {
   ++table_id;
   field_id_t field_id = 0;
   for (const auto &field: db.get_fields(table.first))
   {
    ++field_id;
    Type type = db.get_field_type(table.first, field.first);
    if (type.get_type_id() == Type::type_id_t::reference)
     type = Type::reference(table_map[type.get_table_id()]);
    field_maps[table.first][field.first] = field_id;

    writeable.add_field(table_map[table.first], field.second, type);
   }
  }
 }

 //
 // Dump records
 //
 for (auto table: db.get_tables())
 {
  const table_id_t table_id = table.first;
  const record_id_t last_record_id = db.get_last_record_id(table_id);

  record_id_t record_id = 1;

  while (record_id <= last_record_id)
  {
   while (record_id <= last_record_id &&
          !db.is_used(table_id, record_id))
    record_id++;

   record_id_t size = 0;

   while (record_id + size <= last_record_id &&
          db.is_used(table_id, record_id + size))
    size++;

   if (size)
   {
    writeable.insert_vector(table_map[table_id], record_id, size);
    record_id += size;
   }
  }

  for (const auto &field: db.get_fields(table_id))
  {
   const field_id_t field_id = field.first;

   for (record_id_t record_id = 1; record_id <= last_record_id; record_id++)
    if (db.is_used(table_id, record_id))
    {
     switch(db.get_field_type(table_id, field_id).get_type_id())
     {
      case Type::type_id_t::null:
      break;

      #define TYPE_MACRO(type, return_type, type_id, R, W)\
      case Type::type_id_t::type_id:\
       writeable.update_##type_id(table_map[table_id], record_id, field_maps[table_id][field_id], db.get_##type_id(table_id, record_id, field_id));\
      break;
      #include "joedb/TYPE_MACRO.h"
      #undef TYPE_MACRO
     }
    }
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
void joedb::dump_data(const Readable &db, Writeable &writeable)
/////////////////////////////////////////////////////////////////////////////
{
 for (auto table: db.get_tables())
 {
  const table_id_t table_id = table.first;
  const record_id_t last_record_id = db.get_last_record_id(table_id);

  record_id_t record_id = 1;

  while (record_id <= last_record_id)
  {
   while (record_id <= last_record_id &&
          !db.is_used(table_id, record_id))
    record_id++;

   record_id_t size = 0;

   while (record_id + size <= last_record_id &&
          db.is_used(table_id, record_id + size))
    size++;

   if (size)
   {
    writeable.insert_vector(table_id, record_id, size);

    for (const auto &field: db.get_fields(table_id))
    {
     const field_id_t field_id = field.first;

     switch(db.get_field_type(table_id, field_id).get_type_id())
     {
      case Type::type_id_t::null:
      break;

      #define TYPE_MACRO(type, return_type, type_id, R, W)\
      case Type::type_id_t::type_id:\
      {\
       std::vector<type> v(size);\
       for (record_id_t i = 0; i < size; i++)\
        v[i] = db.get_##type_id(table_id, record_id + i, field_id);\
       writeable.update_vector_##type_id(table_id, record_id, field_id, size, &v[0]);\
      }\
      break;
      #include "joedb/TYPE_MACRO.h"
      #undef TYPE_MACRO
     }
    }

    record_id += size;
   }
  }
 }
}

#include "Selective_Writeable.h"
#include "Multiplexer.h"
#include "joedb/Journal_File.h"

/////////////////////////////////////////////////////////////////////////////
void joedb::pack(Journal_File &input_journal, Writeable &writeable)
/////////////////////////////////////////////////////////////////////////////
{
 Database db;

 Selective_Writeable schema_filter(writeable, Selective_Writeable::Mode::schema);
 Multiplexer multiplexer;
 multiplexer.add_writeable(db);
 multiplexer.add_writeable(schema_filter);

 input_journal.replay_log(multiplexer);

 dump_data(db, writeable);
}
