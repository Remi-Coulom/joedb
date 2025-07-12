#include "joedb/compiler/generator/Procedures_cpp.h"
#include "joedb/compiler/nested_namespace.h"

namespace joedb::generator
{
 Procedures_cpp::Procedures_cpp
 (
  const Compiler_Options &options,
  const std::vector<Procedure> &procedures
 ):
  Generator(".", "procedures/Procedures.cpp", options),
  procedures(procedures)
 {
 }

 void Procedures_cpp::generate()
 {
  auto name_space = options.get_name_space();
  name_space.emplace_back("procedures");

  out << "\n#include \"Procedures.h\"\n\n";

  namespace_open(out, name_space);

  out << "\n Procedures::Procedures(Service &service):\n";

  {
   bool first = true;
   for (const auto &procedure: procedures)
   {
    if (!first)
     out << ",\n";
    else
     first = false;
    out << "  " << procedure.name << "(service)";
   }
  }

  out << "\n {\n";
  for (const auto &procedure: procedures)
   out << "  procedures.emplace_back(&" << procedure.name << ");\n";
  out << '\n';
  for (const auto &procedure: procedures)
   out << "  names.emplace_back(\"" << procedure.name << "\");\n";

  out << " }\n";

  namespace_close(out, name_space);
 }
}
