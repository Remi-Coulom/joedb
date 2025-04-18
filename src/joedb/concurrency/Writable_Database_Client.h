#ifndef joedb_Writable_Database_Client_declared
#define joedb_Writable_Database_Client_declared

#include "joedb/concurrency/Client.h"
#include "joedb/interpreted/Database.h"
#include "joedb/Multiplexer.h"

namespace joedb
{
 namespace detail
 {
  class Writable_Database_Client_Data
  {
   protected:
    Writable_Journal journal;
    Database database;
    Multiplexer multiplexer;

   public:
    Writable_Database_Client_Data(Buffered_File &file):
     journal(file),
     multiplexer{journal, database}
    {
    }
  };
 };

 /// @ingroup concurrency
 class Writable_Database_Client:
  protected detail::Writable_Database_Client_Data,
  public Client
 {
  friend class Writable_Database_Client_Lock;

  protected:
   void read_journal() override {journal.play_until_checkpoint(database);}

  public:
   Writable_Database_Client
   (
    Buffered_File &file,
    Connection &connection,
    bool content_check = true
   ):
    Writable_Database_Client_Data(file),
    Client(journal, connection, content_check)
   {
    read_journal();
   }

   const Database &get_database() const
   {
    return database;
   }

   template<typename F> void transaction(F transaction)
   {
    Client::transaction([this, &transaction]()
    {
     transaction(database, multiplexer);
    });
   }
 };

 /// @ingroup concurrency
 class Writable_Database_Client_Lock: public Client_Lock
 {
  public:
   Writable_Database_Client_Lock(Writable_Database_Client &client):
    Client_Lock(client)
   {
   }

   const Readable &get_readable() const
   {
    return static_cast<Writable_Database_Client &>(client).database;
   }

   Writable &get_writable()
   {
    JOEDB_ASSERT(is_locked());
    return static_cast<Writable_Database_Client &>(client).multiplexer;
   }
 };
}

#endif
