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
  protected:
   Database database;

  public:
   const Database &get_database() const
   {
    return database;
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Writable_Interpreted_Client_Data: public Interpreted_Client_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Writable_Journal journal;
   Multiplexer multiplexer;

  public:
   Writable_Interpreted_Client_Data(Generic_File &file):
    journal(file),
    multiplexer{database, journal}
   {
   }

   Multiplexer &get_multiplexer()
   {
    return multiplexer;
   }

   bool is_readonly() const final {return false;}

   Writable_Journal &get_writable_journal() final
   {
    return journal;
   }

   void update() final
   {
    journal.play_until_checkpoint(database);
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Readonly_Interpreted_Client_Data: public Interpreted_Client_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Readonly_Journal journal;

  public:
   Readonly_Interpreted_Client_Data(Generic_File &file):
    journal(file)
   {
   }

   bool is_readonly() const final {return true;}

   Readonly_Journal &get_readonly_journal() final
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
