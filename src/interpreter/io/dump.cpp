#include "dump.h"
#include "Database.h"

#include <iostream>

using namespace joedb;

void joedb::dump(std::ostream &out, const Database &database)
{
 auto tables = database.get_tables();

 //
 // Dump tables
 //
 for (auto table: tables)
  out << "create_table " << table.second.get_name() <<
         " # id = " << table.first << '\n';

 //
 // Dump fields
 //
 for (auto table: tables)
 {
  const auto &fields = table.second.get_fields();

  for (const auto &field: fields)
  {
   out << "add_field " << table.second.get_name() << ' ';
   out << field.second.name << ' ';

   switch(field.second.type.get_kind())
   {
    case Type::string_id:
     out << "string";
    break;

    case Type::int32_id:
     out << "int32";
    break;

    case Type::int64_id:
     out << "int64";
    break;

    case Type::reference_id:
    {
     out << "references ";
     table_id_t table_id = field.second.type.get_table_id();
     const auto it = tables.find(table_id);
     if (it != tables.end())
      out << it->second.get_name();
     else
      out << "a_deleted_table";
    }
    break;
   }

   out << " # id = " << field.first << "; index = " << field.second.index;
   out << '\n';
  }
 }

 //
 // Dump records
 //
 for (auto table: tables)
 {
  const auto &fields = table.second.get_fields();

  for (auto record: table.second.get_records())
  {
   out << "insert_into " << table.second.get_name() << ' ';
   out << record.first;
   for (const auto &field: fields)
   {
    const size_t i = field.second.index;
    out << ' ';

    if (!record.second[i].is_initialized())
     out << "NULL";
    else
     switch(field.second.type.get_kind())
     {
      case Type::string_id:
       out << record.second[i].get_string();
      break;

      case Type::int32_id:
       out << record.second[i].get_int32();
      break;

      case Type::int64_id:
       out << record.second[i].get_int64();
      break;

      case Type::reference_id:
       out << record.second[i].get_record_id();
      break;
     }
   }
   out << '\n';
  }
 }
}
