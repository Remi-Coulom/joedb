#ifndef joedb_Readonly_Database_Client_declared
#define joedb_Readonly_Database_Client_declared

#include "joedb/concurrency/Readonly_Client.h"
#include "joedb/interpreted/Database.h"

namespace joedb
{
 /// @ingroup concurrency
 class Readonly_Database_Client: public Readonly_Client
 {
  private:
    Database database;

  protected:
   void read_journal() override
   {
    Readonly_Journal::play_until_checkpoint(database);
   }

  public:
   Readonly_Database_Client
   (
    Buffered_File &file,
    Connection &connection,
    Content_Check content_check = Content_Check::quick
   ):
    Readonly_Client(file, connection, content_check)
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
