#ifndef joedb_Journal_Client_declared
#define joedb_Journal_Client_declared

#include "joedb/concurrency/Client.h"
#include "joedb/Multiplexer.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Journal_Client_Data
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

   Writable_Journal &get_journal()
   {
    return journal;
   }

   const Readonly_Journal &get_journal() const
   {
    return journal;
   }

   void update()
   {
    journal.seek_to_checkpoint();
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Journal_Client: public Client<Journal_Client_Data>
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   Journal_Client
   (
    Connection &connection,
    Generic_File &file
   ):
    Client(connection, file)
   {
   }

   template<typename F> void transaction(F transaction)
   {
    Client::transaction([&]()
    {
     transaction(data.journal);
    });
   }
 };
}

#endif
