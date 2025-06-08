#include "joedb/compiler/generator/Procedure_h.h"
#include "joedb/compiler/nested_namespace.h"

namespace joedb::generator
{
 Procedure_h::Procedure_h(const Compiler_Options &options):
  Generator(".", "Procedure.h", options)
 {
 }

 void Procedure_h::generate()
 {
  namespace_include_guard(out, "Procedure", options.get_name_space());

  out << '\n';
  out << "#include \"joedb/rpc/Procedure.h\"\n";
  out << "#include \"Writable_Database.h\"\n";
  out << '\n';

  namespace_open(out, options.get_name_space());

  out << R"RRR(
 class Procedure: public joedb::rpc::Procedure
 {
  public:
   Procedure(std::string_view name):
    joedb::rpc::Procedure
    (
     name,
     std::string_view(detail::schema_string, detail::schema_string_size)
    )
   {
   }

   virtual void execute(Writable_Database &db) = 0;

   void execute(joedb::Buffered_File &file) override
   {
    Writable_Database db(file, joedb::Recovery::ignore_header);
    execute(db);
    db.soft_checkpoint();
   }
 };
)RRR";

  namespace_close(out, options.get_name_space());

  out << "\n#endif\n";
 }
}
