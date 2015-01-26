#include "dump.h"
#include "Database.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
void joedb::dump(std::ostream &out, const Database &database)
{
 auto tables = database.get_tables();

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

   switch(field.second.get_type().get_type_id())
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
     table_id_t table_id = field.second.get_type().get_table_id();
     const auto it = tables.find(table_id);
     if (it != tables.end())
      out << it->second.get_name();
     else
      out << "a_deleted_table";
    }
    break;
   }

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
      out << table.second.get_string(record_id, field.first);
     break;

     case Type::type_id_t::int32:
     break;

     case Type::type_id_t::int64:
     break;

     case Type::type_id_t::reference:
     break;
    }
   }
   out << '\n';
  }
 }
}
