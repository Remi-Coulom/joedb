#ifndef joedb_Interpreted_Client_declared
#define joedb_Interpreted_Client_declared

#include "joedb/concurrency/Client.h"
#include "joedb/interpreter/Database.h"
#include "joedb/Multiplexer.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_Client_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   Writable_Journal journal;
   Database database;

   Interpreted_Client_Data(Generic_File &local_file): journal(local_file)
   {
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_Client: public Interpreted_Client_Data, public Client
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Multiplexer multiplexer;

  public:
   Interpreted_Client
   (
    Connection &connection,
    Generic_File &local_file
   ):
    Interpreted_Client_Data(local_file),
    Client(connection, Interpreted_Client_Data::journal, database),
    multiplexer{database, Interpreted_Client_Data::journal}
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
