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
   explicit Disconnection(const char *what_arg): Exception(what_arg)
   {
   }

   explicit Disconnection(const std::string &what_arg): Exception(what_arg)
   {
   }
 };
}

#endif
