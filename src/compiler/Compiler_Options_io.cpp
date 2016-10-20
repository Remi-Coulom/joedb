#include "Compiler_Options_io.h"
#include "Compiler_Options.h"
#include "is_identifier.h"
#include "Database.h"

#include <iostream>
#include <sstream>

/////////////////////////////////////////////////////////////////////////////
bool joedb::parse_compiler_options
/////////////////////////////////////////////////////////////////////////////
(
 std::istream &in,
 std::ostream &out,
 Compiler_Options &compiler_options
)
{
 std::string line;

 while(std::getline(in, line))
 {
  std::istringstream iss(line);
  std::string command;
  iss >> command;

  const Database &db = compiler_options.get_db();

  if (command.size() == 0 || command[0] == '#')
   continue;
  if (command == "namespace")
  {
   std::string namespace_name;
   iss >> namespace_name;
   compiler_options.set_namespace_name(namespace_name);
  }
  else if (command == "create_index" || command == "create_unique_index")
  {
   Compiler_Options::Index index;

   index.unique = (command == "create_unique_index");

   std::string index_table;
   std::vector<std::string> index_columns;

   iss >> index.name >> index_table;
   {
    std::string s;
    iss >> s;
    std::istringstream column_ss(s);
    std::string column;
    while (std::getline(column_ss, column, ','))
     index_columns.push_back(column);
   }

   if (!joedb::is_identifier(index.name))
   {
    out << "Error: invalid index name: " << index.name << '\n';
    return false;
   }

   index.table_id = db.find_table(index_table);
   if (!index.table_id)
   {
    out << "Error: no such table: " << index_table << '\n';
    return false;
   }

   for (auto field_name: index_columns)
   {
    field_id_t field_id = db.find_field(index.table_id, field_name);
    if (!field_id)
    {
     out << "Error: no such field: " << field_name << '\n';
     return false;
    }
    index.field_ids.push_back(field_id);
   }

   compiler_options.add_index(index);
  }
  else
  {
   out << "unknown command: " << command << '\n';
   return false;
  }
 }

 return true;
}
