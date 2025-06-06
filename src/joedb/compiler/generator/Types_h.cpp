#include "joedb/compiler/generator/Types_h.h"
#include "joedb/compiler/nested_namespace.h"

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 Types_h::Types_h
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options
 ):
  Generator(".", "Types.h", options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Types_h::generate()
 ////////////////////////////////////////////////////////////////////////////
 {
  const Database &db = options.get_db();
  auto tables = db.get_tables();

  namespace_include_guard(out, "Types", options.get_name_space());

  out << R"RRR(
#include "Database.h"

)RRR";

  namespace_open(out, options.get_name_space());

  static constexpr const char * const type_names[] =
  {
   "Writable_Database",
   "Client",
   "Database",
   "File_Client",
   "File_Database",
   "Interpreted_File_Database",
   "Readable",
   "Readonly_Client",
   "Readonly_Database",
   "Readonly_Interpreted_File_Database",
  };

  out << '\n';
  for (const char * const type_name: type_names)
   out << " class " << type_name << ";\n";

  out << R"RRR(
 /// All types defined for this database, listed in a class (for use as template parameter)
 class Types
 {
  public:
)RRR";

  const std::string ns = namespace_string(options.get_name_space()) + "::";

  for (const char * const type_name: type_names)
   out << "   using " << type_name << " = " << ns << type_name << ";\n";

  for (const auto &[tid, tname]: tables)
  {
   out << '\n';
   out << "   using " << "id_of_" << tname;
   out << " = " << ns << "id_of_" << tname << ";\n";
   out << "   using iterator_of_" << tname;
   out << " = " << ns << "container_of_" << tname << "::iterator;\n";
  }

  out << " };\n";

  namespace_close(out, options.get_name_space());
  out << "\n#endif\n";
 }
}
