#ifndef joedb_Interpreted_Client_Data_declared
#define joedb_Interpreted_Client_Data_declared

#include "joedb/concurrency/Client_Data.h"
#include "joedb/interpreter/Database.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/Multiplexer.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_Client_Data: public Client_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Writable_Journal journal;
   Database database;
   Multiplexer multiplexer;

  public:
   Interpreted_Client_Data(Generic_File &file):
    journal(file),
    multiplexer{database, journal}
   {
   }

   Database &get_database()
   {
    return database;
   }

   Multiplexer &get_multiplexer()
   {
    return multiplexer;
   }

   Writable_Journal &get_journal() final
   {
    return journal;
   }

   void update() final
   {
    journal.play_until_checkpoint(database);
   }
 };
}

#endif
