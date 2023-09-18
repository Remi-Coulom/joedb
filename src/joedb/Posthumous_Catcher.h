#ifndef joedb_Posthumous_Catcher_declared
#define joedb_Posthumous_Catcher_declared

#include "joedb/Exception.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Posthumous_Catcher
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Posthumous_Thrower;

  private:
   std::exception_ptr exception;
   void catch_current_exception(const char *message) noexcept;

  public:
   void rethrow();
 };
}

#endif
