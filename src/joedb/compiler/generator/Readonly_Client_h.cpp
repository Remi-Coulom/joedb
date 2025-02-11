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
 ////////////////////////////////////////////////////////////////////////////
 class Readonly_Client_Data:
 ////////////////////////////////////////////////////////////////////////////
  public joedb::Client_Data,
  public joedb::Blob_Reader
 {
  protected:
   joedb::Readonly_Journal journal;
   Database db;

   Readonly_Client_Data(joedb::File &file):
    journal(file)
   {
    db.initialize_with_readonly_journal(journal);
   }

   bool is_readonly() const override
   {
    return true;
   }

   joedb::Readonly_Journal &get_readonly_journal() override
   {
    return journal;
   }

   std::string read_blob_data(joedb::Blob blob) final
   {
    return journal.read_blob_data(blob);
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Pullonly_Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   joedb::Pullonly_Connection connection;
 };

 ////////////////////////////////////////////////////////////////////////////
 class Readonly_Client:
 ////////////////////////////////////////////////////////////////////////////
  private Readonly_Client_Data,
  private Pullonly_Connection,
  private joedb::Pullonly_Client
 {
  private:
   const int64_t schema_checkpoint;

  public:
   Readonly_Client(joedb::File &file):
    Readonly_Client_Data(file),
    joedb::Pullonly_Client
    (
     *static_cast<Readonly_Client_Data *>(this),
     Pullonly_Connection::connection,
     false
    ),
    schema_checkpoint(db.get_schema_checkpoint())
   {
   }

   const Database &get_database() const {return db;}

   bool pull()
   {
    joedb::Pullonly_Client::pull();
    if (journal.get_position() < journal.get_checkpoint_position())
    {
     journal.play_until_checkpoint(db);
     if (db.get_schema_checkpoint() > schema_checkpoint)
      Database::throw_exception("Pulled a schema change");
     return true;
    }
    else
     return false;
   }
 };
)RRR";

  namespace_close(out, options.get_name_space());
  out << "\n#endif\n";  
 }
}
