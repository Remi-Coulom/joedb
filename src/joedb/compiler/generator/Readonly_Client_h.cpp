#include "joedb/compiler/generator/Readonly_Client_h.h"
#include "joedb/compiler/nested_namespace.h"

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 Readonly_Client_h::Readonly_Client_h
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options
 ):
  Generator(".", "Readonly_Client.h", options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Readonly_Client_h::write(std::ostream &out)
 ////////////////////////////////////////////////////////////////////////////
 {
  namespace_include_guard_open(out, "Readonly_Client", options.get_name_space());

  out << R"RRR(
#include "Database_Writable.h"
#include "joedb/concurrency/Readonly_Client.h"

)RRR";

  namespace_open(out, options.get_name_space());

  out << R"RRR(
 /// Client for a read-only file (allows pulling, unlike @ref Readonly_Database)
 class Readonly_Client: public joedb::Readonly_Client
 {
  private:
   Database_Writable db;
   int64_t schema_checkpoint;

  protected:
   virtual void read_journal() override
   {
    Readonly_Journal::play_until_checkpoint(db);
    if (db.get_schema_checkpoint() > schema_checkpoint)
     Database::throw_exception("Pulled a schema change");
   }

  public:
   Readonly_Client(joedb::Abstract_File &file):
    joedb::Readonly_Client
    (
     file,
     joedb::Connection::dummy,
     joedb::Content_Check::none
    )
   {
    db.initialize_with_readonly_journal(*this);
    schema_checkpoint = db.get_schema_checkpoint();
   }

   using joedb::Readonly_Client::pull;

   const Database &get_database() const {return db;}
 };
)RRR";


  namespace_close(out, options.get_name_space());
  namespace_include_guard_close(out);
 }
}
