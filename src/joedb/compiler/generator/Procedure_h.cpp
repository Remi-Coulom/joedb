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
#include "../../Client.h"

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
   virtual void execute(Writable_Database &message) = 0;

   void execute(joedb::Buffered_File &file) override
   {
    Writable_Database db(file, joedb::Recovery::ignore_header);
    execute(db);
    db.soft_checkpoint();
   }

  public:
   Procedure(): joedb::rpc::Procedure
   (
    std::string(detail::schema_string, detail::schema_string_size)
   )
   {
   }

   void execute_locally(Memory_Database &message)
   {
    message.soft_checkpoint();
    joedb::File_View file_view(message.get_file_view());
    execute(file_view);
    message.pull();
   }
 };

 /// Function type for procedures that write to the database
 typedef void (*Write_Function)
 (
  )RRR" << ns << R"RRR(::Writable_Database &db,
  Writable_Database &message
 );

 /// Wrapper for procedures that write to the database
 class Write_Procedure: public Procedure
 {
  private:
   )RRR" << ns << R"RRR(::Client &client;
   const Write_Function function;

   void execute(Writable_Database &message) override
   {
    client.transaction([&]()RRR" << ns << R"RRR(::Writable_Database &db)
    {
     function(db, message);
    });
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

 /// Function type for const procedures
 typedef void (*Read_Function)
 (
  const )RRR" << ns << R"RRR(::Database &db,
  Writable_Database &message
 );

 /// Wrapper for const procedures
 class Read_Procedure: public Procedure
 {
  private:
   const )RRR" << ns << R"RRR(::Database &db;
   const Read_Function function;

   void execute(Writable_Database &message) override
   {
    function(db, message);
   }

  public:
   Read_Procedure
   (
    const )RRR" << ns << R"RRR(::Database &db,
    Read_Function function
   ):
    db(db),
    function(function)
   {
   }
 };
)RRR";

  namespace_close(out, options.get_name_space());

  out << "\n#endif\n";
 }
}
