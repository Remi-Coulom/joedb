#include "joedb/compiler/generator/Client_h.h"
#include "joedb/compiler/nested_namespace.h"

namespace joedb::generator
{
 ////////////////////////////////////////////////////////////////////////////
 Client_h::Client_h
 ////////////////////////////////////////////////////////////////////////////
 (
  const Compiler_Options &options
 ):
  Generator(".", "Client.h", options)
 {
 }

 ////////////////////////////////////////////////////////////////////////////
 void Client_h::generate()
 ////////////////////////////////////////////////////////////////////////////
 {
  namespace_include_guard(out, "Client", options.get_name_space());

  out << R"RRR(
#include "Writable_Database.h"
#include "joedb/concurrency/Client.h"

)RRR";

  namespace_open(out, options.get_name_space());
 out << R"RRR(
 namespace detail
 {
  ///////////////////////////////////////////////////////////////////////////
  class Client_Data: public joedb::Client_Data
  ///////////////////////////////////////////////////////////////////////////
  {
   protected:
    Writable_Database db;

    Client_Data
    (
     joedb::Buffered_File &file,
     joedb::Readonly_Journal::Check check,
     joedb::Commit_Level commit_level
    ):
     db(file, false, check, commit_level)
    {
    }

    bool is_readonly() const final
    {
     return false;
    }

    joedb::Writable_Journal &get_writable_journal() final
    {
     return db.journal;
    }
  };
 }

 /// Handle concurrent access to a @ref joedb::Buffered_File using a @ref joedb::Connection
 class Client:
  protected detail::Client_Data,
  public joedb::Client,
  public joedb::Blob_Reader
 {
  private:
   int64_t schema_checkpoint;

   void play_journal_and_throw_if_schema_changed()
   {
    db.play_journal();
    if (db.schema_journal.get_checkpoint_position() > schema_checkpoint)
     Database::throw_exception("Can't upgrade schema during pull");
    db.check_single_row();
   }

  public:
   Client
   (
    joedb::Buffered_File &file,
    joedb::Connection &connection,
    bool content_check = true,
    joedb::Readonly_Journal::Check check = joedb::Readonly_Journal::Check::all,
    joedb::Commit_Level commit_level = joedb::Commit_Level::no_commit
   ):
    detail::Client_Data(file, check, commit_level),
    joedb::Client(*static_cast<joedb::Client_Data *>(this), connection, content_check)
   {
    if (get_checkpoint_difference() > 0)
     push_unlock();

    db.play_journal(); // makes transaction shorter if db is big
    joedb::Client::transaction([this](joedb::Client_Data &data){
     db.initialize();
    });

    schema_checkpoint = db.schema_journal.get_checkpoint_position();
   }

   const Database &get_database() const
   {
    return db;
   }

   std::string read_blob_data(joedb::Blob blob) final
   {
    return db.read_blob_data(blob);
   }

   int64_t pull(std::chrono::milliseconds wait = std::chrono::milliseconds(0))
   {
    const int64_t byte_count = joedb::Client::pull(wait);
    play_journal_and_throw_if_schema_changed();
    return byte_count;
   }

   /// Execute a write transaction
   template<typename F> void transaction
   (
    F transaction ///< A function object that takes a reference to a @ref Writable_Database
   )
   {
    joedb::Client::transaction([&](joedb::Client_Data &data)
    {
     play_journal_and_throw_if_schema_changed();
     transaction(db);
    });
   }
 };
)RRR";

  namespace_close(out, options.get_name_space());
  out << "\n#endif\n";
 }
}
