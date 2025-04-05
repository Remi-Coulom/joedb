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
   Writable_Journal &get_writable_journal() override
   {
    return journal;
   }

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
   }

   bool is_readonly() const override {return false;}

   template<typename F> void transaction(F transaction)
   {
    Client::transaction([this, &transaction]()
    {
     journal.play_until_checkpoint(database);
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
    client.journal.play_until_checkpoint(client.database);
   }

   const Database &get_database() const
   {
    return static_cast<Writable_Database_Client &>(client).database;
   }

   Multiplexer &get_multiplexer()
   {
    return static_cast<Writable_Database_Client &>(client).multiplexer;
   }
 };
}

#endif
