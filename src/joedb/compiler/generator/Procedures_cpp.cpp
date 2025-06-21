#include "joedb/compiler/generator/Procedures_cpp.h"
#include "joedb/compiler/nested_namespace.h"

#include <set>

namespace joedb::generator
{
 Procedures_cpp::Procedures_cpp
 (
  const Compiler_Options &options,
  const std::vector<Procedure> &procedures
 ):
  Generator(".", "Procedures.cpp", options),
  procedures(procedures)
 {
 }

 void Procedures_cpp::generate()
 {
  out << "#include \"Procedures.h\"\n\n";

  {
   std::set<std::string> includes;

   for (const auto &procedure: procedures)
    includes.insert(procedure.include);

   for (const auto &include: includes)
    out << "#include \"" << include << "\"\n";

   out << '\n';
  }

  namespace_open(out, options.get_name_space());


  out << "\n Procedures::Procedures(Client &client):\n";

  {
   bool first = true;
   for (const auto &procedure: procedures)
   {
    if (!first)
     out << ",\n";
    else
     first = false;
    out << "  " << procedure.name << "(client";
    if (procedure.type == Procedure::read)
     out << ".get_database()";
    out << ", procedures::" << procedure.name << ')';
   }
  }

  out << "\n {\n";
  out << " }\n";

  namespace_close(out, options.get_name_space());
 }
}
