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
 void Readonly_Client_h::generate()
 ////////////////////////////////////////////////////////////////////////////
 {
  namespace_include_guard(out, "Readonly_Client", options.get_name_space());

  out << R"RRR(
#include "Database.h"
#include "joedb/concurrency/Client.h"
#include "joedb/journal/File.h"

)RRR";

  namespace_open(out, options.get_name_space());

  out << R"RRR(
 namespace detail
 {
  class Readonly_Client_Data
  {
   protected:
    joedb::Connection connection;
    joedb::Readonly_Journal journal;
    Database db;

    Readonly_Client_Data(joedb::File &file): journal(file)
    {
     db.initialize_with_readonly_journal(journal);
    }
  };
 }

 /// Client for a read-only file (allows pulling, unlike @ref Readonly_Database)
 class Readonly_Client:
  private detail::Readonly_Client_Data,
  public joedb::Client
 {
  private:
   const int64_t schema_checkpoint;

   bool is_readonly() const override
   {
    return true;
   }

   joedb::Readonly_Journal &get_readonly_journal() override
   {
    return journal;
   }

  public:
   Readonly_Client(joedb::File &file):
    detail::Readonly_Client_Data(file),
    joedb::Client
    (
     journal,
     Readonly_Client_Data::connection,
     false
    ),
    schema_checkpoint(db.get_schema_checkpoint())
   {
   }

   const Database &get_database()
   {
    journal.play_until_checkpoint(db);
    if (db.get_schema_checkpoint() > schema_checkpoint)
     Database::throw_exception("Pulled a schema change");
    return db;
   }
 };
)RRR";

  namespace_close(out, options.get_name_space());
  out << "\n#endif\n";
 }
}
