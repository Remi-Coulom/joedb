#include "joedb/compiler/generator/struct_h.h"
#include "joedb/compiler/nested_namespace.h"

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 struct_h::struct_h
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options,
  const std::pair<Table_Id, std::string> &table
 ):
  Generator("structs", (table.second + ".h").c_str(), options),
  table(table)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void struct_h::generate()
 ////////////////////////////////////////////////////////////////////////////
 {
  const Table_Id tid = table.first;
  const std::string &table_name = table.second;
  const auto &db = options.db;

  namespace_include_guard
  (
   out,
   ("structs_" + table_name).c_str(),
   options.get_name_space()
  );

  out << '\n';

  {
   bool has_reference = false;
   bool has_string = false;

   for (const auto &[fid, fname]: db.get_fields(table.first))
   {
    const joedb::Type &type = db.get_field_type(tid, fid);

    if (type.get_type_id() == Type::Type_Id::reference)
     has_reference = true;

    if (type.get_type_id() == Type::Type_Id::string)
     has_string = true;
   }

   if (has_reference)
    out << "#include \"../ids.h\"\n\n";

   if (has_string)
    out << "#include <string>\n\n";
  }

  namespace_open(out, options.get_name_space());

  out << "\n struct " << table_name << '\n';
  out << " {\n";

  for (const auto &[fid, fname]: db.get_fields(table.first))
  {
   const joedb::Type &type = db.get_field_type(tid, fid);
   out << "  ";
   write_type(type, false, false);
   out << fname << ";\n";
  }

  // TODO: struct_tt (bool instead of char for boolean)
  // TODO: default value

  out << " };\n";

  namespace_close(out, options.get_name_space());

  out << "\n#endif\n";  
 }
}
