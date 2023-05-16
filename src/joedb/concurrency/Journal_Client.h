#ifndef joedb_Journal_Client_declared
#define joedb_Journal_Client_declared

#include "joedb/concurrency/Client.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Journal_Client_Data: public Client_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Journal_Client;

  private:
   Writable_Journal journal;

  public:
   Journal_Client_Data
   (
    joedb::Connection &connection,
    Generic_File &file
   ):
    journal(file)
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

   void update() final
   {
    journal.seek_to_checkpoint();
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Journal_Client: private Journal_Client_Data, public Client
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   Journal_Client
   (
    Connection &connection,
    Generic_File &file
   ):
    Journal_Client_Data(connection, file),
    Client(connection, *this)
   {
   }

   template<typename F> void transaction(F transaction)
   {
    Client::transaction([&]()
    {
     transaction(journal);
    });
   }
 };
}

#endif
