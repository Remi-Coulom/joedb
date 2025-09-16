#include "joedb/compiler/generator/Procedure_h.h"
#include "joedb/compiler/nested_namespace.h"

namespace joedb::generator
{
 Procedure_h::Procedure_h
 (
  const Compiler_Options &options,
  const Compiler_Options &parent_options
 ):
  Generator(".", "Procedure.h", options),
  parent_options(parent_options)
 {
 }

 void Procedure_h::write(std::ostream &out)
 {
  namespace_include_guard_open(out, "Procedure", options.get_name_space());

  out << '\n';
  if (&options == &parent_options)
   out << "#include \"rpc/Service.h\"";
  else
   out << "#include \"../Service.h\"";
  out << R"RRR(
#include "joedb/rpc/Procedure.h"
#include "Writable_Database.h"

)RRR";


  namespace_open(out, options.get_name_space());

  out << R"RRR(
 /// Class for all procedures based on this message schema
 class Procedure: public joedb::rpc::Procedure
 {
  protected:
   rpc::Service &service;

   virtual void execute(Writable_Database &message) const = 0;

   void execute(joedb::Abstract_File &file) const override
   {
    Writable_Database db(file, joedb::Recovery::ignore_header);
    execute(db);
    db.soft_checkpoint();
   }

  public:
   Procedure(rpc::Service &service): service(service)
   {
   }
 };
)RRR";

  namespace_close(out, options.get_name_space());
  namespace_include_guard_close(out);
 }
}
