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
   Readonly_Journal &get_readonly_journal() override
   {
    return journal;
   }

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
   }

   const Database &get_database()
   {
    journal.play_until_checkpoint(database);
    return database;
   }

   bool is_readonly() const override {return true;}
 };
}

#endif
