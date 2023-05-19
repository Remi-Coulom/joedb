#ifndef joedb_Journal_Client_Data_declared
#define joedb_Journal_Client_Data_declared

#include "joedb/concurrency/Client_Data.h"
#include "joedb/journal/Writable_Journal.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Journal_Client_Data: public Client_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Writable_Journal journal;

  public:
   Journal_Client_Data(Generic_File &file): journal(file)
   {
   }

   Writable_Journal &get_journal() final
   {
    return journal;
   }

   void update() final
   {
   }
 };
}

#endif
