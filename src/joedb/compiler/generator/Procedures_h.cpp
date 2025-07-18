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
  Generator(".", "rpc/Procedures.h", options),
  procedures(procedures)
 {
 }

 void Procedures_h::generate()
 {
  auto name_space = options.get_name_space();
  name_space.emplace_back("rpc");

  namespace_include_guard(out, "Procedures", name_space);
  out << '\n';

  {
   std::set<std::string> schemas;

   for (const auto &procedure: procedures)
    schemas.insert(procedure.schema);

   for (const auto &schema: schemas)
    out << "#include \"" << options.get_path(schema) << "/Procedure.h\"\n";
  }

  out << '\n';
  namespace_open(out, name_space);

  out << R"RRR(
 /// A collection of procedures to be used by joedb::rpc::Server
 class Procedures
 {
  public:)RRR";

  for (const auto &procedure: procedures)
  {
   const auto &name = procedure.name;
   const auto &schema = procedure.schema;

   out << R"RRR(
   class )RRR" << name << R"RRR(: public )RRR" << schema << R"RRR(::Procedure
   {
    private:
     void execute()RRR" << schema << R"RRR(::Writable_Database &message) const override
     {
      service.)RRR" << name << R"RRR((message);
     }

    public:
     )RRR" << name << R"RRR((Service &service): )RRR" << schema << R"RRR(::Procedure(service) {}
   } )RRR" << name << R"RRR(;
)RRR";
  }

  out << "\n   const std::vector<joedb::rpc::Procedure *> procedures\n";
  out << "   {\n";

  for (const auto &procedure: procedures)
   out << "    &" << procedure.name << ",\n";

  out << "   };\n\n";

  out << "   Procedures(Service &service):\n";

  {
   bool first = true;
   for (const auto &procedure: procedures)
   {
    if (first)
     first = false;
    else
     out << ",\n";
    out << "    " << procedure.name << "(service)";
   }
  }

  out << "\n   {\n";
  out << "   }\n";
  out << " };\n";

  namespace_close(out, name_space);
  out << "\n#endif\n";
 }
}
