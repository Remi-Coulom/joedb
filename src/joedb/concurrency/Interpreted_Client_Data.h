#ifndef joedb_Interpreted_Client_Data_declared
#define joedb_Interpreted_Client_Data_declared

#include "joedb/concurrency/Client_Data.h"
#include "joedb/interpreter/Database.h"
#include "joedb/journal/Writable_Journal.h"
#include "joedb/Multiplexer.h"

#include <memory>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_Client_Data: public Client_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Writable_Journal journal;
   Database database;
   Multiplexer transaction_multiplexer;
   std::unique_ptr<Multiplexer> update_multiplexer;

  public:
   Interpreted_Client_Data
   (
    Generic_File &file,
    Writable *update_writable = nullptr
   ):
    journal(file),
    transaction_multiplexer{database, journal}
   {
    if (update_writable)
     update_multiplexer.reset(new Multiplexer{database, *update_writable});
   }

   const Database &get_database() const
   {
    return database;
   }

   Multiplexer &get_multiplexer()
   {
    return transaction_multiplexer;
   }

   Writable_Journal &get_journal() final
   {
    return journal;
   }

   void update() final
   {
    if (update_multiplexer)
     journal.play_until_checkpoint(*update_multiplexer);
    else
     journal.play_until_checkpoint(database);
   }
 };
}

#endif
