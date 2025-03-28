#ifndef joedb_Writable_Journal_Client_Data_declared
#define joedb_Writable_Journal_Client_Data_declared

#include "joedb/concurrency/Client_Data.h"
#include "joedb/journal/Writable_Journal.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Writable_Journal_Client_Data: public Client_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Writable_Journal journal;

  public:
   Writable_Journal_Client_Data(Buffered_File &file): journal(file)
   {
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
}

#endif
