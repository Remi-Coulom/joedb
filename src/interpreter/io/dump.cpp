#include "dump.h"
#include "joedb/Database.h"
#include "joedb/Listener.h"

/////////////////////////////////////////////////////////////////////////////
void joedb::dump(const Database &db, Listener &listener)
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
   listener.after_create_table(table.second.get_name());
   table_map[table.first] = table_id;
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
   for (const auto &field: table.second.get_fields())
   {
    ++field_id;
    auto type = field.second.get_type();
    if (type.get_type_id() == Type::type_id_t::reference)
     type = Type::reference(table_map[type.get_table_id()]);
    listener.after_add_field(table_map[table.first],
                             field.second.get_name(),
                             type);
    field_maps[table.first][field.first] = field_id;
   }
  }
 }

 //
 // Dump records
 //
 for (auto table: db.get_tables())
 {
  const auto &fields = table.second.get_fields();
  const auto &freedom = table.second.get_freedom();

  size_t i = 0;

  while (i < freedom.size())
  {
   while (i < freedom.size() && freedom.is_free(i + 2))
    i++;
   size_t size = 0;
   while (i + size < freedom.size() && !freedom.is_free(i + 2 + size))
    size++;

   if (size)
   {
    listener.after_insert_vector(table_map[table.first], i + 1, size);
    i += size;
   }
  }

  for (const auto &field: fields)
  {
   for (size_t i = 0; i < freedom.size(); i++)
    if (!freedom.is_free(i + 2))
    {
     record_id_t record_id = i + 1;

     switch(field.second.get_type().get_type_id())
     {
      case Type::type_id_t::null:
      break;

      #define TYPE_MACRO(type, return_type, type_id, R, W)\
      case Type::type_id_t::type_id:\
       listener.after_update_##type_id(table_map[table.first], record_id, field_maps[table.first][field.first], table.second.get_##type_id(record_id, field.first));\
      break;
      #include "joedb/TYPE_MACRO.h"
      #undef TYPE_MACRO
     }
    }
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
void joedb::dump_data(const Database &db, Listener &listener)
/////////////////////////////////////////////////////////////////////////////
{
 for (auto table: db.get_tables())
 {
  const auto &fields = table.second.get_fields();
  const auto &freedom = table.second.get_freedom();

  size_t i = 0;

  while (i < freedom.size())
  {
   while (i < freedom.size() && freedom.is_free(i + 2))
    i++;
   size_t size = 0;
   while (i + size < freedom.size() && !freedom.is_free(i + 2 + size))
    size++;

   if (size)
   {
    listener.after_insert_vector(table.first, i + 1, size);

    for (const auto &field: fields)
    {
     switch(field.second.get_type().get_type_id())
     {
      case Type::type_id_t::null:
      break;

      #define TYPE_MACRO(type, return_type, type_id, R, W)\
      case Type::type_id_t::type_id:\
       listener.after_update_vector_##type_id(table.first, i + 1, field.first, size, field.second.get_vector_##type_id() + i);\
      break;
      #include "joedb/TYPE_MACRO.h"
      #undef TYPE_MACRO
     }
    }

    i += size;
   }
  }
 }
}

#include "DB_Listener.h"
#include "Selective_Listener.h"
#include "Multiplexer.h"
#include "joedb/Journal_File.h"

/////////////////////////////////////////////////////////////////////////////
void joedb::pack(Journal_File &input_journal, Listener &listener)
/////////////////////////////////////////////////////////////////////////////
{
 Database db;
 DB_Listener db_listener(db);

 Selective_Listener schema_writer(listener, Selective_Listener::Mode::schema);
 Multiplexer multiplexer;
 multiplexer.add_listener(db_listener);
 multiplexer.add_listener(schema_writer);
 Dummy_Listener dummy;
 auto &multiplexer_listener = multiplexer.add_listener(dummy);

 input_journal.replay_log(multiplexer_listener);

 dump_data(db, listener);
}
