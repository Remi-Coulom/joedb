#include "joedb/Posthumous_Thrower.h"
#include "joedb/Posthumous_Catcher.h"
#include "joedb/Destructor_Logger.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Posthumous_Thrower::postpone_exception(const char *message) noexcept
 ////////////////////////////////////////////////////////////////////////////
 {
  if (catcher)
   catcher->catch_current_exception(message);
  if (message)
   Destructor_Logger::write(message);
 }
}
