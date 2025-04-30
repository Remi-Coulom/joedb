#ifndef joedb_Readonly_Database_Client_declared
#define joedb_Readonly_Database_Client_declared

#include "joedb/concurrency/Readonly_Client.h"
#include "joedb/interpreted/Database.h"

namespace joedb
{
 namespace detail
 {
  class Readonly_Database_Client_Data
  {
   protected:
    Readonly_Journal data_journal;
    Database database;

   public:
    Readonly_Database_Client_Data(Buffered_File &file):
     data_journal(file)
    {
    }
  };
 };

 /// @ingroup concurrency
 class Readonly_Database_Client:
  protected detail::Readonly_Database_Client_Data,
  public Readonly_Client
 {
  protected:
   void read_journal() override {journal.play_until_checkpoint(database);}

  public:
   Readonly_Database_Client
   (
    Buffered_File &file,
    Connection &connection,
    Content_Check content_check = Content_Check::quick
   ):
    Readonly_Database_Client_Data(file),
    Readonly_Client(data_journal, connection, content_check)
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
