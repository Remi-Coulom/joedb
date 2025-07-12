#include "joedb/compiler/generator/Signatures_h.h"
#include "joedb/compiler/nested_namespace.h"

#include <set>

namespace joedb::generator
{
 Signatures_h::Signatures_h
 (
  const Compiler_Options &options,
  const std::vector<Procedure> &procedures
 ):
  Generator(".", "rpc/Signatures.h", options),
  procedures(procedures)
 {
 }

 void Signatures_h::generate()
 {
  auto name_space = options.get_name_space();
  name_space.emplace_back("rpc");

  namespace_include_guard(out, "Signatures", name_space);
  out << "\n#include \"joedb/rpc/Signature.h\"\n";

  {
   std::set<std::string> schemas;

   for (const auto &procedure: procedures)
    schemas.insert(procedure.schema);

   for (const auto &schema: schemas)
    out << "#include \"" << schema << "/Memory_Database.h\"\n";
  }

  out << '\n';
  namespace_open(out, name_space);

  out << R"RRR(
 /// Get the list of procedure signatures. Used by RPC client and server.
 inline const auto &get_signatures()
 {
  static const std::vector<joedb::rpc::Signature> signatures
  {
)RRR";

  for (const auto &procedure: procedures)
  {
   out << "   {\"" << procedure.name << "\", " << procedure.schema;
   out << "::Memory_Database().get_data()},\n";
  }

  out << "  };\n\n";
  out << "  return signatures;\n";
  out << " }\n";

  namespace_close(out, name_space);
  out << "\n#endif\n";
 }
}
