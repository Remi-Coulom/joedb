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
  else if (command == "create_index")
  {
   std::string index_name;
   std::string index_table;
   std::vector<std::string> index_columns;
   std::string index_type_name;

   iss >> index_name >> index_table;
   {
    std::string s;
    iss >> s;
    std::istringstream column_ss(s);
    std::string column;
    while (std::getline(column_ss, column, ','))
     index_columns.push_back(column);
   }
   iss >> index_type_name;

   if (!joedb::is_identifier(index_name))
   {
    out << "Error: invalid index name: " << index_name << '\n';
    return false;
   }

   table_id_t table_id = db.find_table(index_table);
   if (!table_id)
   {
    out << "Error: no such table: " << index_table << '\n';
    return false;
   }

   std::vector<field_id_t> field_ids;
   for (auto field_name: index_columns)
   {
    field_id_t field_id = db.find_field(table_id, field_name);
    if (!field_id)
    {
     out << "Error: no such field: " << field_name << '\n';
     return false;
    }
    field_ids.push_back(field_id);
   }

   Compiler_Options::index_type_t index_type;
   if (index_type_name == "map")
    index_type = Compiler_Options::index_type_t::map;
   else if (index_type_name == "multimap")
    index_type = Compiler_Options::index_type_t::multimap;
   else if (index_type_name == "unordered_map")
    index_type = Compiler_Options::index_type_t::unordered_map;
   else if (index_type_name == "unordered_multimap")
    index_type = Compiler_Options::index_type_t::unordered_multimap;
   else
   {
    out << "unknown index type: " << index_type_name << '\n';
    return false;
   }
  }
  else
  {
   out << "unknown command: " << command << '\n';
   return false;
  }
 }

 return true;
}
