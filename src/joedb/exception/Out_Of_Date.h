#ifndef joedb_exception_Out_Of_Date_declared
#define joedb_exception_Out_Of_Date_declared

#include "joedb/Exception.h"

namespace joedb::exception
{
 ////////////////////////////////////////////////////////////////////////////
 class Out_Of_Date: public Exception
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   Out_Of_Date(const std::string &message): Exception(message)
   {
   }
 };
}

#endif
