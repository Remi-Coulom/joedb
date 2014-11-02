#include "dump.h"
#include "Database.h"

#include <iostream>

using namespace crazydb;

void crazydb::dump(std::ostream &out, const Database &database)
{
 auto tables = database.get_tables();
 for (auto table: tables)
 {
  out << "create_table " << table.first << '\n';
  auto field_names = table.second.get_field_names();
  auto field_types = table.second.get_field_types();

  for (size_t i = 0; i < field_names.size(); i++)
  {
   out << "add_field " << table.first << ' ' << field_names[i] << ' ';

   switch(field_types[i].get_kind())
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
     out << "references " << field_types[i].get_table_name();
    break;
   }
   out << '\n';
  }

  for (auto record: table.second.get_records())
  {
   out << "insert_into " << table.first << ' ';
   out << record.first;
   for (size_t i = 0; i < field_names.size(); i++)
   {
    out << ' ';

    if (!record.second[i].is_initialized())
     out << "NULL";
    else
     switch(field_types[i].get_kind())
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

  out << '\n';
 }
}
