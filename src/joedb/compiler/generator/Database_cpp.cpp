#include "joedb/compiler/generator/Database_cpp.h"
#include "joedb/compiler/nested_namespace.h"
#include "joedb/ui/type_io.h"

namespace joedb::compiler::generator
{
 ////////////////////////////////////////////////////////////////////////////
 Database_cpp::Database_cpp
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options
 ):
  Generator(".", "Database.cpp", options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Database_cpp::generate()
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "#include \"Database.h\"\n\n";

  namespace_open(out, options.get_name_space());

  out << "\n const char * schema_string = ";
  ui::write_string(out, options.schema_file.get_data());
  out << ";\n";  

  namespace_close(out, options.get_name_space());
 }
}
