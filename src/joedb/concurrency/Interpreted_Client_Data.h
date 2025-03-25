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
   interpreter::Database database;

  public:
   virtual const interpreter::Database &get_database() = 0;
 };

 ////////////////////////////////////////////////////////////////////////////
 class Writable_Interpreted_Client_Data: public Interpreted_Client_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Writable_Journal journal;
   Multiplexer multiplexer;

  public:
   Writable_Interpreted_Client_Data(Buffered_File &file):
    journal(file),
    multiplexer{database, journal}
   {
   }

   Multiplexer &get_multiplexer()
   {
    journal.play_until_checkpoint(database);
    return multiplexer;
   }

   const interpreter::Database &get_database() final
   {
    journal.play_until_checkpoint(database);
    return database;
   }

   bool is_readonly() const final
   {
    return false;
   }

   Writable_Journal &get_writable_journal() final
   {
    return journal;
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Readonly_Interpreted_Client_Data: public Interpreted_Client_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Readonly_Journal journal;

  public:
   Readonly_Interpreted_Client_Data(Buffered_File &file):
    journal(file)
   {
   }

   bool is_readonly() const final
   {
    return true;
   }

   Readonly_Journal &get_readonly_journal() final
   {
    return journal;
   }

   const interpreter::Database &get_database() final
   {
    journal.play_until_checkpoint(database);
    return database;
   }
 };
}

#endif
