#include "parse_schema.h"

#include <sstream>

#include "Schema.h"

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
