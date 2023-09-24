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
   virtual Writable_Journal &get_journal() = 0;
   virtual void update() = 0;
   virtual ~Client_Data();
 };
}

#endif
