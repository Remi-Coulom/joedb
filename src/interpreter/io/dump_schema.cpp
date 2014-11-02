#include "dump_schema.h"
#include "Database.h"

#include <iostream>

using namespace crazydb;

void crazydb::dump_schema(std::ostream &out, const Database &database)
{
 auto tables = database.get_tables();
 for (auto table_it: tables)
 {
  out << "create_table " << table_it.first << '\n';
  auto fields = table_it.second.get_fields();
  for (auto field_it: fields)
  {
   out << "add_field " << table_it.first << ' ' << field_it.first << ' ';

   switch(field_it.second.get_kind())
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
     out << "references " << field_it.second.get_table_name();
     break;
   }

   out << '\n';
  }
  out << '\n';
 }
}
