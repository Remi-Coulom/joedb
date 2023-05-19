#ifndef joedb_Journal_Client_declared
#define joedb_Journal_Client_declared

#include "joedb/concurrency/Client.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Journal_Client: private Client_Data, public Client
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Writable_Journal &client_journal;

  public:
   Journal_Client(Connection &connection):
    Client(connection, *this),
    client_journal(connection.client_journal)
   {
   }

   template<typename F> void transaction(F transaction)
   {
    Client::transaction([&]()
    {
     transaction(client_journal);
    });
   }
 };
}

#endif
