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
 };

 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_Lock
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Interpreted_Client &interpreted_client;
   Client_Write_Lock lock;

  public:
   Interpreted_Lock(Interpreted_Client &interpreted_client):
    interpreted_client(interpreted_client),
    lock(interpreted_client.client)
   {
   }

   Readable_Writable &get_database()
   {
    return interpreted_client.multiplexer;
   }
 };
}

#endif
