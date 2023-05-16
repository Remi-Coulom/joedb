#ifndef joedb_Interpreted_Client_declared
#define joedb_Interpreted_Client_declared

#include "joedb/concurrency/Client.h"
#include "joedb/interpreter/Database.h"
#include "joedb/Multiplexer.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_Client_Data: public Client_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Interpreted_Client;

  private:
   Writable_Journal journal;
   Database database;
   Multiplexer multiplexer;

  public:
   Interpreted_Client_Data
   (
    joedb::Connection &connection,
    Generic_File &file
   ):
    journal(file),
    multiplexer{database, journal}
   {
   }

   Writable_Journal &get_journal() final
   {
    return journal;
   }

   const Readonly_Journal &get_journal() const final
   {
    return journal;
   }

   void update()
   {
    journal.play_until_checkpoint(database);
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_Client:
 ////////////////////////////////////////////////////////////////////////////
  private Interpreted_Client_Data,
  public Client
 {
  public:
   Interpreted_Client
   (
    Connection &connection,
    Generic_File &file
   ):
    Interpreted_Client_Data(connection, file),
    Client(connection, *this)
   {
   }

   const Readable &get_database() const
   {
    return database;
   }

   template<typename F> void transaction(F transaction)
   {
    Client::transaction([&]()
    {
     transaction(database, multiplexer);
    });
   }
 };
}

#endif
