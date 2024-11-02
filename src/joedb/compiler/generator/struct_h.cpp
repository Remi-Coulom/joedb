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
  namespace_include_guard
  (
   out,
   ("structs_" + table.second).c_str(),
   options.get_name_space()
  );

  out << '\n';

  namespace_open(out, options.get_name_space());

  out << "\n// Hello\n";

  namespace_close(out, options.get_name_space());

  out << "\n#endif\n";  
 }
}
