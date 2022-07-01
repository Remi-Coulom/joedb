#ifndef joedb_exception_Out_Of_Date_declared
#define joedb_exception_Out_Of_Date_declared

#include "joedb/Exception.h"

namespace joedb
{
 namespace exception
 {
  ////////////////////////////////////////////////////////////////////////////
  class Out_Of_Date: public Exception
  ////////////////////////////////////////////////////////////////////////////
  {
   public:
    Out_Of_Date(): Exception
    (
     "Schema is out of date. Can't upgrade a read-only database."
    )
    {}
  };
 }
}

#endif
