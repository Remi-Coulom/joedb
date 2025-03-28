#ifndef joedb_Out_Of_Date_declared
#define joedb_Out_Of_Date_declared

#include "joedb/error/Exception.h"

namespace joedb
{
 /// sent when the schema of a read-only compiled database is out of date
 /// \ingroup error
 class Out_Of_Date: public Exception
 {
  public:
   Out_Of_Date(const std::string &message): Exception(message)
   {
   }
 };
}

#endif
