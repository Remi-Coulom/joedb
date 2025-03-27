#include "joedb/error/Posthumous_Thrower.h"
#include "joedb/error/Posthumous_Catcher.h"
#include "joedb/error/Destructor_Logger.h"

namespace joedb::error
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
