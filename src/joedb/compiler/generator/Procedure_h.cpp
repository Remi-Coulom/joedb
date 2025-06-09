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

  const std::string ns = namespace_string
  (
   parent_options.get_name_space()
  );

  out << R"RRR(
 class Procedure: public joedb::rpc::Procedure
 {
  private:
   virtual void execute(Writable_Database &population) = 0;

   void execute(joedb::Buffered_File &file) override
   {
    Writable_Database db(file, joedb::Recovery::ignore_header);
    execute(db);
    db.soft_checkpoint();
   }

  public:
   Procedure(): joedb::rpc::Procedure
   (
    std::string_view(detail::schema_string, detail::schema_string_size)
   )
   {
   }
 };

 typedef void (*Write_Function)
 (
  )RRR" << ns << R"RRR(::Client &client,
  Writable_Database &population
 );

 class Write_Procedure: public Procedure
 {
  private:
   )RRR" << ns << R"RRR(::Client &client;
   const Write_Function function;

   void execute(Writable_Database &population) override
   {
    function(client, population);
   }

  public:
   Write_Procedure
   (
    )RRR" << ns << R"RRR(::Client &client,
    Write_Function function
   ):
    client(client),
    function(function)
   {
   }
 };

 typedef void (*Read_Function)
 (
  const )RRR" << ns << R"RRR(::Database &db,
  Writable_Database &population
 );

 class Read_Procedure: public Procedure
 {
  private:
   )RRR" << ns << R"RRR(::Client &client;
   const Read_Function function;

   void execute(Writable_Database &population) override
   {
    client.pull();
    function(client.get_database(), population);
   }

  public:
   Read_Procedure
   (
    )RRR" << ns << R"RRR(::Client &client,
    Read_Function function
   ):
    client(client),
    function(function)
   {
   }
 };
)RRR";

  namespace_close(out, options.get_name_space());

  out << "\n#endif\n";
 }
}
