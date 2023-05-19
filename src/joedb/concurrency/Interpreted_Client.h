#ifndef joedb_Interpreted_Client_declared
#define joedb_Interpreted_Client_declared

#include "joedb/interpreter/Database.h"
#include "joedb/journal/Readonly_Journal.h"
#include "joedb/concurrency/Client.h"
#include "joedb/Multiplexer.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_Client_Data: public Client_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   Database database;

  public:
   const Readable &get_database() const
   {
    return database;
   }

   void update(Readonly_Journal &journal)
   {
    journal.play_until_checkpoint(database);
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_Client:
 ////////////////////////////////////////////////////////////////////////////
  public Interpreted_Client_Data,
  public Client
 {
  private:
   Multiplexer multiplexer;

  public:
   Interpreted_Client
   (
    Connection &connection
   ):
    Client(connection, *static_cast<Interpreted_Client_Data *>(this)),
    multiplexer{database, connection.client_journal}
   {
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
