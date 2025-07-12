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
  Generator(".", "procedures/Procedures.h", options),
  procedures(procedures)
 {
 }

 void Procedures_h::generate()
 {
  auto name_space = options.get_name_space();
  name_space.emplace_back("procedures");

  namespace_include_guard(out, "Procedures", name_space);
  out << '\n';

  {
   std::set<std::string> schemas;

   for (const auto &procedure: procedures)
    schemas.insert(procedure.schema);

   for (const auto &schema: schemas)
    out << "#include \"" << schema << "/Procedure.h\"\n";

   out << "\n#include \"joedb/rpc/Procedures.h\"\n\n";
  }

  namespace_open(out, name_space);

  out << R"RRR(
 class Procedures: public joedb::rpc::Procedures
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
     void execute
     (
      Service &service,
      )RRR" << schema << R"RRR(::Writable_Database &message
     ) override
     {
      service.)RRR" << name << R"RRR((message);
     }

    public:
     )RRR" << name << R"RRR((Service &service): )RRR" << schema << R"RRR(::Procedure(service) {}
   } )RRR" << name << R"RRR(;
)RRR";
  }

  out << R"RRR(
   Procedures(Service &service);
  };
)RRR";


  namespace_close(out, name_space);
  out << "\n#endif\n";
 }
}
