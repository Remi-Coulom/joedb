#ifndef joedb_Disconnection_declared
#define joedb_Disconnection_declared

#include "joedb/error/Exception.h"

namespace joedb
{
 /// @ref joedb::Robust_Connection does not try to reconnect when thrown
 /// @ingroup error
 class Disconnection: public Exception
 {
  public:
   Disconnection(const std::string &message): Exception(message)
   {
   }
 };
}

#endif
