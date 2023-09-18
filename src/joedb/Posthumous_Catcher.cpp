#include "joedb/Posthumous_Catcher.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 void Posthumous_Catcher::catch_current_exception(const char *message) noexcept
 ////////////////////////////////////////////////////////////////////////////
 {
  if (!exception)
  {
   exception = std::current_exception();
   if (!exception)
    try
    {
     throw Exception(message);
    }
    catch (...)
    {
     exception = std::current_exception();
    }
  }
 }

 ////////////////////////////////////////////////////////////////////////////
 void Posthumous_Catcher::rethrow()
 ////////////////////////////////////////////////////////////////////////////
 {
  if (exception)
   std::rethrow_exception(exception);
 }
}
