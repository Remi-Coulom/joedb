#ifndef joedb_Interpreted_Client_declared
#define joedb_Interpreted_Client_declared

#include "joedb/concurrency/Client.h"
#include "joedb/interpreter/Database.h"
#include "joedb/Readable_Multiplexer.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_Client
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Interpreted_Lock;

  private:
   Writable_Journal journal;
   Database database;
   Readable_Multiplexer multiplexer;
   Client client;

  public:
   Interpreted_Client
   (
    Connection &connection,
    Generic_File &local_file
   ):
    journal(local_file),
    multiplexer(database),
    client(connection, journal, database)
   {
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
    std::function<void(Readable_Writable&)> transaction
   )
   {
    client.write_transaction
    (
     [&]()
     {
      transaction(multiplexer);
     }
    );
   }
 };
}

#endif
