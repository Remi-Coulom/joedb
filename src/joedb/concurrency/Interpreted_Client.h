#ifndef joedb_Interpreted_Client_declared
#define joedb_Interpreted_Client_declared

#include "joedb/concurrency/Connection.h"
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
   Connection_Control control;

  public:
   Interpreted_Client
   (
    Connection &connection,
    Generic_File &local_file
   ):
    journal(local_file),
    multiplexer(database),
    control(connection, journal, database)
   {
    multiplexer.add_writable(journal);
   }

   Readable &get_database()
   {
    return database;
   }

   int64_t pull() {return control.pull();}
 };

 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_Lock
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Interpreted_Client &client;
   Lock lock;

  public:
   Interpreted_Lock(Interpreted_Client &client):
    client(client),
    lock(client.control)
   {
   }

   Readable_Writable &get_database()
   {
    return client.multiplexer;
   }
 };
}

#endif
