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
  const auto &fields = table.second.get_fields();

  for (const auto &field: fields)
  {
   out << "add_field " << table.first << ' ' << field.second.name << ' ';

   switch(field.second.type.get_kind())
   {
    case Type::null_id:
     out << "null";
    break;

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
     out << "references " << field.second.type.get_table_name();
    break;
   }

   out << " # id = " << field.first << "; index = " << field.second.index;
   out << '\n';
  }

  for (auto record: table.second.get_records())
  {
   out << "insert_into " << table.first << ' ';
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
      case Type::null_id:
       out << "NULL";
      break;

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
