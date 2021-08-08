#ifndef joedb_Posthumous_Thrower_declared
#define joedb_Posthumous_Thrower_declared

#include <exception>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Posthumous_Catcher
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Posthumous_Thrower;

  private:
   std::exception_ptr exception;

   void catch_current_exception() noexcept
   {
    exception = std::current_exception();
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
   void postpone_exception() noexcept
   {
    if (catcher)
     catcher->catch_current_exception();
   }

  public:
   void set_catcher(Posthumous_Catcher &catcher) noexcept
   {
    this->catcher = &catcher;
   }
 };
}

#endif
