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
    client(connection, journal, database)
   {
    multiplexer.add_writable(database);
    multiplexer.add_writable(journal);
   }

   Readable &get_database()
   {
    return database;
   }

   int64_t pull()
   {
    return client.pull();
   }

   void write_transaction
   (
    std::function<void(Readable&, Writable&)> transaction
   )
   {
    client.write_transaction
    (
     [&]()
     {
      transaction(database, multiplexer);
     }
    );
   }
 };
}

#endif
