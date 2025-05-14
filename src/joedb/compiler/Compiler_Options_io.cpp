#include "joedb/compiler/Compiler_Options_io.h"
#include "joedb/compiler/Compiler_Options.h"
#include "joedb/is_identifier.h"
#include "joedb/interpreted/Database_Schema.h"
#include "joedb/compiler/nested_namespace.h"
#include "joedb/ui/type_io.h"
#include "joedb/ui/Readable_Command_Processor.h"

#include <iostream>
#include <sstream>

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 void parse_compiler_options
 /////////////////////////////////////////////////////////////////////////////
 (
  std::istream &in,
  Compiler_Options &compiler_options
 )
 {
  std::string line;

  while(std::getline(in, line))
  {
   std::istringstream iss(line);
   std::string command;
   iss >> command;

   const Database_Schema &db = compiler_options.get_db();

   if (command.empty() || command[0] == '#')
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

    iss >> index.name;

    index.table_id = Readable_Command_Processor::parse_table(iss, db);

    {
     std::string s;
     iss >> s;
     std::istringstream column_ss(s);

     while (true)
     {
      std::string field_name;
      if (std::getline(column_ss, field_name, ','))
      {
       const Field_Id field_id = db.find_field(index.table_id, field_name);
       if (field_id == Field_Id(0))
        throw Exception("Field not found: " + field_name);
       index.field_ids.emplace_back(field_id);
      }
      else
       break;
     }
    }

    if (!joedb::is_identifier(index.name))
     throw Exception("Invalid index identifier: " + index.name);

    compiler_options.add_index(std::move(index));
   }
   else if (command == "set_table_storage")
   {
    std::cerr << "Warning: set_table_storage is deprecated\n";
   }
   else if (command == "set_single_row")
   {
    const Table_Id table_id = Readable_Command_Processor::parse_table(iss, db);
    const bool value = read_boolean(iss);
    compiler_options.set_single_row(table_id, value);
   }
   else
    throw Exception("unknown command: " + command);
  }
 }
}
