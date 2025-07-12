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

 void Procedure_h::generate()
 {
  namespace_include_guard(out, "Procedure", options.get_name_space());

  out << R"RRR(
#include "joedb/rpc/Procedure.h"
#include "Writable_Database.h"
#include "Memory_Database.h"
#include "../Service.h"

)RRR";

  namespace_open(out, options.get_name_space());

  const std::string ns = namespace_string
  (
   parent_options.get_name_space()
  );

  out << R"RRR(
 /// Class for all procedures based on this message schema
 class Procedure: public joedb::rpc::Procedure
 {
  private:
   Service &service;

   virtual void execute(Service &service, Writable_Database &message) = 0;

   void execute(joedb::Buffered_File &file) override
   {
    Writable_Database db(file, joedb::Recovery::ignore_header);
    execute(service, db);
    db.soft_checkpoint();
   }

  public:
   Procedure(Service &service):
    joedb::rpc::Procedure(Memory_Database().get_data()),
    service(service)
   {
   }
 };
)RRR";

  namespace_close(out, options.get_name_space());

  out << "\n#endif\n";
 }
}
