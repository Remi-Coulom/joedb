#include "dump.h"
#include "Database.h"
#include "string_io.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
void joedb::write_type(std::ostream &out, const Database &db, Type type)
{
 switch(type.get_type_id())
 {
  case Type::type_id_t::null:
   out << "null";
  break;

  case Type::type_id_t::string:
   out << "string";
  break;

  case Type::type_id_t::int32:
   out << "int32";
  break;

  case Type::type_id_t::int64:
   out << "int64";
  break;

  case Type::type_id_t::reference:
  {
   out << "references ";
   table_id_t table_id = type.get_table_id();
   const auto it = db.get_tables().find(table_id);
   if (it != db.get_tables().end())
    out << it->second.get_name();
   else
    out << "a_deleted_table";
  }
  break;

  case Type::type_id_t::boolean:
   out << "boolean";
  break;
 }
}

/////////////////////////////////////////////////////////////////////////////
void joedb::dump(std::ostream &out, const Database &db)
{
 auto tables = db.get_tables();

 //
 // Dump tables
 //
 for (auto table: tables)
  out << "create_table " << table.second.get_name() << '\n';

 //
 // Dump fields
 //
 for (auto table: tables)
 {
  const auto &fields = table.second.get_fields();

  for (const auto &field: fields)
  {
   out << "add_field " << table.second.get_name() << ' ';
   out << field.second.get_name() << ' ';
   write_type(out, db, field.second.get_type());
   out << '\n';
  }
 }

 //
 // Dump records
 //
 for (auto table: tables)
 {
  const auto &fields = table.second.get_fields();

  const auto &freedom = table.second.get_freedom();

  for (size_t i = freedom.get_first_used(); i != 0; i = freedom.get_next(i))
  {
   record_id_t record_id = i - 1;
   out << "insert_into " << table.second.get_name() << ' ';
   out << record_id;
   for (const auto &field: fields)
   {
    out << ' ';

    switch(field.second.get_type().get_type_id())
    {
     case Type::type_id_t::null:
      out << "NULL";
     break;

     case Type::type_id_t::string:
      joedb::write_string(out, table.second.get_string(record_id, field.first));
     break;

#define FIELD_CASE(type)\
     case Type::type_id_t::type:\
      out << table.second.get_##type(record_id, field.first);\
     break;

     FIELD_CASE(int32)
     FIELD_CASE(int64)
     FIELD_CASE(reference)
     FIELD_CASE(boolean)

#undef FIELD_CASE
    }
   }
   out << '\n';
  }
 }
}
