#ifndef joedb_Client_Data_declared
#define joedb_Client_Data_declared

#include "joedb/journal/Writable_Journal.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Client_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   virtual bool is_readonly() const = 0;
   virtual Writable_Journal &get_writable_journal();
   virtual Readonly_Journal &get_readonly_journal();
   virtual ~Client_Data();

   bool has_aborted_transaction()
   {
    const Readonly_Journal &journal = get_readonly_journal();
    return journal.get_position() > journal.get_checkpoint_position();
   }
 };
}

#endif
