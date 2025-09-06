#include "joedb/compiler/generator/Database_Writable_cpp.h"
#include "joedb/compiler/nested_namespace.h"
#include "joedb/ui/type_io.h"

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 Database_Writable_cpp::Database_Writable_cpp
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options
 ):
  Generator(".", "Database_Writable.cpp", options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Database_Writable_cpp::generate()
 ////////////////////////////////////////////////////////////////////////////
 {
  out << "#include \"Database_Writable.h\"\n\n";

  auto name_space = options.get_name_space();
  name_space.emplace_back("detail");

  namespace_open(out, name_space);

  out << "\n const char * schema_string = ";
  write_string(out, options.schema_file.get_data());
  out << ";\n";

  namespace_close(out, name_space);
  out.flush();
 }
}
