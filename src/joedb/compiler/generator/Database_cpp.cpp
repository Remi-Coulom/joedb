#include "joedb/compiler/generator/Database_cpp.h"
#include "joedb/compiler/nested_namespace.h"
#include "joedb/ui/type_io.h"

namespace joedb::generator
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

  auto name_space = options.get_name_space();
  name_space.emplace_back("detail");

  namespace_open(out, name_space);

  out << "\n const char * schema_string = ";
  write_string(out, options.schema_file.get_data());
  out << ";\n";

  namespace_close(out, name_space);
 }
}
