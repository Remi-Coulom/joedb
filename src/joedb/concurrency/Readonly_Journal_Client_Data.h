#ifndef joedb_Readonly_Journal_Client_Data_declared
#define joedb_Readonly_Journal_Client_Data_declared

#include "joedb/concurrency/Client_Data.h"
#include "joedb/journal/Readonly_Journal.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Readonly_Journal_Client_Data: public Client_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Readonly_Journal journal;

  public:
   Readonly_Journal_Client_Data(Buffered_File &file): journal(file)
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
 };
}

#endif
