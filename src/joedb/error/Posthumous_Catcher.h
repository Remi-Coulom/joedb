#ifndef joedb_Posthumous_Catcher_declared
#define joedb_Posthumous_Catcher_declared

#include <exception>

namespace joedb
{
 /// Catch exceptions sent from the destructor of a @ref Posthumous_Thrower
 /// \ingroup error
 class Posthumous_Catcher
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
