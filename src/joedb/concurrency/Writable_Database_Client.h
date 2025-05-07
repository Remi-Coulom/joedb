#ifndef joedb_Writable_Database_Client_declared
#define joedb_Writable_Database_Client_declared

#include "joedb/concurrency/Writable_Client.h"
#include "joedb/interpreted/Database.h"
#include "joedb/Multiplexer.h"

namespace joedb
{
 namespace detail
 {
  class Writable_Database_Client_Data
  {
   protected:
    Writable_Journal data_journal;
    Database database;
    Multiplexer multiplexer;

   public:
    Writable_Database_Client_Data(Buffered_File &file):
     data_journal(file),
     multiplexer{database, data_journal}
    {
    }
  };
 };

 /// @ingroup concurrency
 class Writable_Database_Client:
  protected detail::Writable_Database_Client_Data,
  public Writable_Client
 {
  friend class Writable_Database_Client_Lock;

  protected:
   void read_journal() override
   {
    data_journal.play_until_checkpoint(database);
   }

  public:
   Writable_Database_Client
   (
    Buffered_File &file,
    Connection &connection,
    Content_Check content_check = Content_Check::quick
   ):
    Writable_Database_Client_Data(file),
    Writable_Client(data_journal, connection, content_check)
   {
    read_journal();
   }

   const Database &get_database() const
   {
    return database;
   }

   template<typename F> auto transaction(F transaction)
   {
    return Writable_Client::transaction([this, &transaction]()
    {
     return transaction(database, multiplexer);
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
    JOEDB_DEBUG_ASSERT(locked);
    return static_cast<Writable_Database_Client &>(client).multiplexer;
   }
 };
}

#endif
