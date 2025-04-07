#ifndef joedb_Readonly_Database_Client_declared
#define joedb_Readonly_Database_Client_declared

#include "joedb/concurrency/Client.h"
#include "joedb/interpreted/Database.h"

namespace joedb
{
 namespace detail
 {
  class Readonly_Database_Client_Data
  {
   protected:
    Readonly_Journal journal;
    Database database;

   public:
    Readonly_Database_Client_Data(Buffered_File &file):
     journal(file)
    {
    }
  };
 };

 /// @ingroup concurrency
 class Readonly_Database_Client:
  protected detail::Readonly_Database_Client_Data,
  public Client
 {
  protected:
   void read_journal() override {journal.play_until_checkpoint(database);}

  public:
   Readonly_Database_Client
   (
    Buffered_File &file,
    Connection &connection,
    bool content_check = true
   ):
    Readonly_Database_Client_Data(file),
    Client(journal, connection, content_check)
   {
    read_journal();
   }

   const Database &get_database() const
   {
    return database;
   }
 };
}

#endif
