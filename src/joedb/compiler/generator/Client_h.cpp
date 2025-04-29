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
  class Client_Data
  ///////////////////////////////////////////////////////////////////////////
  {
   protected:
    Writable_Database db;

    Client_Data(joedb::Buffered_File &file): db(file, false)
    {
    }
  };
 }

 /// Handle concurrent access to a @ref joedb::Buffered_File using a @ref joedb::Connection
 class Client:
  protected detail::Client_Data,
  public joedb::Client
 {
  friend class Client_Lock;

  private:
   int64_t schema_checkpoint;

  protected:
   void read_journal() override
   {
    db.play_journal();
    if (schema_checkpoint)
    {
     if (db.schema_journal.get_checkpoint() > schema_checkpoint)
      Database::throw_exception("Can't upgrade schema during pull");
     db.check_single_row();
    }
   }

  public:
   Client
   (
    joedb::Buffered_File &file,
    joedb::Connection &connection,
    joedb::Content_Check content_check = joedb::Content_Check::quick
   ):
    detail::Client_Data(file),
    joedb::Client(db.journal, connection, content_check),
    schema_checkpoint(0)
   {
    if (get_checkpoint_difference() > 0)
     push_unlock();

    db.play_journal(); // makes transaction shorter if db is big
    joedb::Client::transaction([this](){
     db.initialize();
    });

    schema_checkpoint = db.schema_journal.get_checkpoint();
   }

   const Database &get_database() const
   {
    return db;
   }

   /// Execute a write transaction
   ///
   /// This function can be called with a lambda like this:
   /// @code
   /// client.transaction([](Writable_Database &db)
   /// {
   ///  db.write_comment("Hello");
   /// });
   /// @endcode
   /// The transaction function locks and pulls the connection before
   /// executing the lambda, pushes and unlocks it after.
   template<typename F> auto transaction
   (
    F transaction
   )
   {
    return joedb::Client::transaction([&]()
    {
     return transaction(db);
    });
   }
 };

 /// For more flexibility than the transaction lambda
 ///
 /// See joedb::Client_Lock for more information
 ///
 /// @include client_lock.cpp
 class Client_Lock: public joedb::Client_Lock
 {
  public:
   Client_Lock(Client &client): joedb::Client_Lock(client)
   {
   }

   Writable_Database &get_database()
   {
    JOEDB_DEBUG_ASSERT(is_locked());
    return static_cast<Client &>(client).db;
   }
 };
)RRR";

  namespace_close(out, options.get_name_space());
  out << "\n#endif\n";
 }
}
