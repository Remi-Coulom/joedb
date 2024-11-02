#include "joedb/compiler/generator/Database_cpp.h"
#include "joedb/compiler/nested_namespace.h"
#include "joedb/io/type_io.h"

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

  namespace_open(out, options.get_name_space());

  out << "\n const char * schema_string = ";                                         const std::vector<char> &v = options.schema_file.get_data();
  write_string(out, std::string(v.data(), v.size()));
  out << ";\n";  

  namespace_close(out, options.get_name_space());
 }
}
