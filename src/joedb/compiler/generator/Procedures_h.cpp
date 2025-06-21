#include "joedb/compiler/generator/Procedures_h.h"
#include "joedb/compiler/nested_namespace.h"

#include <set>

namespace joedb::generator
{
 Procedures_h::Procedures_h
 (
  const Compiler_Options &options,
  const std::vector<Procedure> &procedures
 ):
  Generator(".", "Procedures.h", options),
  procedures(procedures)
 {
 }

 void Procedures_h::generate()
 {
  namespace_include_guard(out, "Procedures", options.get_name_space());

  out << "\n#include \"Client.h\"\n\n";

  {
   std::set<std::string> schemas;

   for (const auto &procedure: procedures)
    schemas.insert(procedure.schema);

   for (const auto &schema: schemas)
    out << "#include \"procedures/" << schema << "/Procedure.h\"\n";

   out << '\n';
  }

  namespace_open(out, options.get_name_space());

  out << "\n class Procedures\n";
  out << " {\n";
  out << "  public:\n";

  for (const auto &procedure: procedures)
  {
   out << "   procedures::" << procedure.schema << "::";
   out << (procedure.type == Procedure::read ? "Read" : "Write");
   out << "_Procedure " << procedure.name << ";\n";
  }

  out << '\n';

  out << "   Procedures(Client &client);\n";
  out << " };\n";

  namespace_close(out, options.get_name_space());
  out << "\n#endif\n";
 }
}
