#include "joedb/compiler/generator/introspection_h.h"
#include "joedb/compiler/nested_namespace.h"
#include "joedb/ui/write_value.h"

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 introspection_h::introspection_h
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options,
  const std::pair<Table_Id, std::string> &table
 ):
  Generator("tables", (table.second + "_introspection.h").c_str(), options),
  table(table)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void introspection_h::generate()
 ////////////////////////////////////////////////////////////////////////////
 {
  const Table_Id tid = table.first;
  const auto &db = options.db;

  out << "#ifndef JOEDB_INTROSPECTION\n";

  if (db.get_freedom(tid).get_used_count() > Record_Id{0})
   out << "#define JOEDB_INTROSPECTION(field_type, field_name, initial)\n";
  else
   out << "#define JOEDB_INTROSPECTION(field_type, field_name)\n";

  out << "#endif\n\n";

  for (const auto &[fid, fname]: db.get_fields(table.first))
  {
   out << "JOEDB_INTROSPECTION(";

   const joedb::Type &type = db.get_field_type(tid, fid);

   if (type.get_type_id() == joedb::Type::Type_Id::boolean)
    out << "bool";
   else
   {
    if (type.get_type_id() == joedb::Type::Type_Id::reference)
    {
     namespace_write(out, options.get_name_space());
     out << "::";
    }
    write_type(type, false, false);
   }

   out << ", " << fname;

   if (db.get_freedom(tid).get_used_count() > Record_Id{0})
   {
    out << ", ";
    const Record_Id record_id{db.get_freedom(tid).get_first_used()};
    write_value(out, db, tid, record_id, fid);
   }
   out << ");\n";
  }

  out << "\n#undef JOEDB_INTROSPECTION\n";
 }
}
