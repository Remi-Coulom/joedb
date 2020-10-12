#ifndef joedb_Interpreted_Client_declared
#define joedb_Interpreted_Client_declared

#include "joedb/server/Connection.h"
#include "joedb/interpreter/Database.h"
#include "joedb/Multiplexer.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_Client
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Interpreted_Write_Lock;

  private:
   Journal_File journal;
   Database database;
   Multiplexer multiplexer;
   Connection_Control control;

  public:
   Interpreted_Client
   (
    Connection &connection,
    Generic_File &local_file
   ):
    journal(local_file),
    control(connection, journal, database)
   {
    multiplexer.add_writable(journal);
    multiplexer.add_writable(database);
   }

   const Readable &get_readable()
   {
    return database;
   }

   void pull() {control.pull();}
 };

 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_Write_Lock
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Interpreted_Client &client;
   Write_Lock write_lock;

  public:
   Interpreted_Write_Lock(Interpreted_Client &client):
    client(client),
    write_lock(client.control)
   {
   }

   Writable &get_writable()
   {
    return client.multiplexer;
   }
 };
}

#endif
