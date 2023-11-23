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
   virtual void update() = 0;
   virtual ~Client_Data();
 };
}

#endif
