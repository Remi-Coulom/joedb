#include "parse_schema.h"

#include <sstream>
#include <map>

#include "Schema.h"
#include "Primitive_type.h"

using namespace crazydb;

/////////////////////////////////////////////////////////////////////////////
bool crazydb::parse_schema(std::istream &in, Schema &schema)
/////////////////////////////////////////////////////////////////////////////
{
 //
 // Name of schema
 //
 {
  std::string line;
  if (!std::getline(in, line))
   return false;
  std::istringstream iss(line);
  if (!(iss >> schema.name))
   return false;
 }

 //
 // Loop over tables
 //
 {
  std::string line;
  while (std::getline(in, line))
  {
   Table table;
   std::istringstream iss(line);

   if (iss >> table.name)
   {
    //
    // Loop over fields
    //
    while (std::getline(in, line))
    {
     Field field;
     std::istringstream iss(line);

     if (iss >> field.type_string >> field.name)
      table.fields.push_back(field);
     else
      break;
    }

    schema.tables.push_back(table);
   }
  }
 }

 return true;
}

/////////////////////////////////////////////////////////////////////////////
void crazydb::write_schema(std::ostream &out, const Schema &schema)
/////////////////////////////////////////////////////////////////////////////
{
 out << schema.name << '\n';

 for (std::list<Table>::const_iterator table = schema.tables.begin();
      table != schema.tables.end();
      table++)
 {
  out << "\n " << table->name << '\n';
  
  for (std::list<Field>::const_iterator field = table->fields.begin();
       field != table->fields.end();
       field++)
  {
   out << "  " << field->type_string << ' ' << field->name << '\n';
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
bool crazydb::parse_types(Schema &schema)
/////////////////////////////////////////////////////////////////////////////
{
 //
 // Map of types
 //
 std::map<std::string, Type*> m;

 //
 // Primary types
 //
 Primitive_type string_type("std::string");
 m["string"] = &string_type;
 Primitive_type int32_t_type("int32_t");
 m["int32_t"] = &int32_t_type;

 //
 // Create all table types
 //
 for (std::list<Table>::const_iterator table = schema.tables.begin();
      table != schema.tables.end();
      table++)
 {
  schema.table_types.push_back(Table_type(*table));
  m[table->name] = &schema.table_types.back();
 }

 //
 // Parse all fields
 //
 for (std::list<Table>::const_iterator table = schema.tables.begin();
      table != schema.tables.end();
      table++)
  for (std::list<Field>::const_iterator field = table->fields.begin();
       field != table->fields.end();
       field++)
  {
   std::map<std::string, Type*>::const_iterator i = m.find(field->type_string);
   if (i == m.end())
   {
    std::cerr << "Error: could not find type: " << field->type_string << '\n';
    return false;
   }
  }

 return true;
}
