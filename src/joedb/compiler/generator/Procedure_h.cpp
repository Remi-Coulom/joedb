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

  out << '\n';
  out << "#include \"joedb/rpc/Procedure.h\"\n";
  out << "#include \"Writable_Database.h\"\n";
  out << "#include \"../../Client.h\"\n";
  out << '\n';

  namespace_open(out, options.get_name_space());

  const std::string ns = namespace_string(options.get_name_space());
  const std::string parent_ns = namespace_string
  (
   parent_options.get_name_space()
  );

  out << R"RRR(
 class Procedure
 {
  public:
   virtual void execute()RRR" << parent_ns << R"RRR(::Client &client, Writable_Database &db) = 0;
   virtual ~Procedure() = default;
 };

 namespace detail
 {
  class Erased_Procedure: public joedb::rpc::Procedure
  {
   private:
    )RRR" << parent_ns << R"RRR(::Client &client;
    )RRR" << ns << R"RRR(::Procedure &procedure;

   public:
    Erased_Procedure
    (
     )RRR" << parent_ns << R"RRR(::Client &client,
     )RRR" << ns << R"RRR(::Procedure &procedure
    ):
     joedb::rpc::Procedure
     (
      std::string_view(detail::schema_string, detail::schema_string_size)
     ),
     client(client),
     procedure(procedure)
    {
    }

    void execute(joedb::Buffered_File &file) override
    {
     Writable_Database db(file, joedb::Recovery::ignore_header);
     procedure.execute(client, db);
     db.soft_checkpoint();
    }
  };
 }
)RRR";

  namespace_close(out, options.get_name_space());

  out << "\n#endif\n";
 }
}
