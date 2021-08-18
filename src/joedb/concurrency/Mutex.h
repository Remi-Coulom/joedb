#ifndef joedb_Mutex_declared
#define joedb_Mutex_declared

#include <exception>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Mutex
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   virtual void lock() = 0;
   virtual void unlock() = 0;

   Mutex(const Mutex&) = delete;
   Mutex &operator=(const Mutex&) = delete;

  public:
   Mutex() {}
   virtual ~Mutex() {}

   /////////////////////////////////////////////////////////////////////////
   template<typename F> void run_while_locked(F f)
   /////////////////////////////////////////////////////////////////////////
   {
    std::exception_ptr exception;

    lock();

    try
    {
     f();
    }
    catch (...)
    {
     exception = std::current_exception();
    }

    unlock();

    if (exception)
     std::rethrow_exception(exception);
   }
 };
}

#endif
