#ifndef joedb_Interpreted_Client_declared
#define joedb_Interpreted_Client_declared

#include "joedb/concurrency/Client.h"
#include "joedb/interpreter/Database.h"
#include "joedb/Multiplexer.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_Client
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Writable_Journal journal;
   Database database;
   Multiplexer multiplexer;
   Client client;

  public:
   Interpreted_Client
   (
    Connection &connection,
    Generic_File &local_file
   ):
    journal(local_file),
    multiplexer{database, journal},
    client(connection, journal, database)
   {
   }

   Readable &get_database()
   {
    return database;
   }

   int64_t pull()
   {
    return client.pull();
   }

   template<typename F> void transaction(F transaction)
   {
    client.transaction([&]()
    {
     transaction(database, multiplexer);
    });
   }
 };
}

#endif
