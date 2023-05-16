#include "joedb/io/Readable_Parser.h"
#include "joedb/Readable.h"

#include <sstream>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 Type Readable_Parser::parse_type
 ////////////////////////////////////////////////////////////////////////////
 (
  std::istream &in,
  std::ostream &out
 ) const
 {
  std::string type_name;
  in >> type_name;

  if (type_name == "references")
  {
   std::string table_name;
   in >> table_name;
   const Table_Id table_id = readable.find_table(table_name);
   if (table_id)
    return Type::reference(table_id);
  }

  #define TYPE_MACRO(type, return_type, type_id, read, write)\
  if (type_name == #type_id)\
   return Type::type_id();
  #define TYPE_MACRO_NO_REFERENCE
  #include "joedb/TYPE_MACRO.h"

  throw Exception("unknown type");
 }

 ////////////////////////////////////////////////////////////////////////////
 Table_Id Readable_Parser::parse_table
 ////////////////////////////////////////////////////////////////////////////
 (
  std::istream &in,
  std::ostream &out
 ) const
 {
  std::string table_name;
  in >> table_name;
  const Table_Id table_id = readable.find_table(table_name);
  if (!table_id)
  {
   std::ostringstream error;
   error << "No such table: " << table_name;
   throw Exception(error.str());
  }
  return table_id;
 }
}
