#ifndef joedb_Posthumous_Thrower_declared
#define joedb_Posthumous_Thrower_declared

#include "joedb/Destructor_Logger.h"
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

   void catch_current_exception(const char *message) noexcept
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

  public:
   void rethrow()
   {
    if (exception)
     std::rethrow_exception(exception);
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Posthumous_Thrower
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Posthumous_Catcher *catcher = nullptr;

  protected:
   void postpone_exception(const char *message = nullptr) noexcept
   {
    if (catcher)
     catcher->catch_current_exception(message);
    if (message)
     Destructor_Logger::write(message);
   }

  public:
   void set_catcher(Posthumous_Catcher &catcher) noexcept
   {
    this->catcher = &catcher;
   }
 };
}

#endif
