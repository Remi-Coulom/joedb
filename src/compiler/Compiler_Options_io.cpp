#include "Compiler_Options_io.h"
#include "Compiler_Options.h"
#include "is_identifier.h"
#include "Database.h"
#include "nested_namespace.h"

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
   std::string s;
   iss >> s;
   compiler_options.set_name_space(split_namespace(s));
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
    Field_Id field_id = db.find_field(index.table_id, field_name);
    if (!field_id)
    {
     out << "Error: no such field: " << field_name << '\n';
     return false;
    }
    index.field_ids.push_back(field_id);
   }

   compiler_options.add_index(index);
  }
  else if (command == "generate_c_wrapper")
  {
   compiler_options.set_generate_c_wrapper(true);
  }
  else if (command == "generate_js_wrapper")
  {
   compiler_options.set_generate_js_wrapper(true);
  }
  else if (command == "set_table_storage")
  {
   std::string table_name;
   std::string storage_string;

   iss >> table_name >> storage_string;

   Table_Id table_id = db.find_table(table_name);
   if (!table_id)
   {
    out << "Error: no such table: " << table_name << '\n';
    return false;
   }

   Compiler_Options::Table_Storage storage;
   if (storage_string == "freedom_keeper")
    storage = Compiler_Options::Table_Storage::freedom_keeper;
   else if (storage_string == "vector")
    storage = Compiler_Options::Table_Storage::vector;
   else
   {
    out << "Error: unknown table storage: " << storage_string << '\n';
    return false;
   }

   compiler_options.set_table_storage(table_id, storage);
  }
  else if (command == "set_table_null_initialization")
  {
   std::string table_name;
   std::string null_initialization;
   iss >> table_name >> null_initialization;

   Table_Id table_id = db.find_table(table_name);
   if (!table_id)
   {
    out << "Error: no such table: " << table_name << '\n';
    return false;
   }

   if (null_initialization == "true")
    compiler_options.set_table_null_initialization(table_id, true);
   else if (null_initialization == "false")
    compiler_options.set_table_null_initialization(table_id, false);
  }
  else
  {
   out << "unknown command: " << command << '\n';
   return false;
  }
 }

 return true;
}
