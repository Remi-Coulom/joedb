#ifndef joedb_Mutex_declared
#define joedb_Mutex_declared

#include <functional>
#include <exception>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Mutex
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Mutex_Lock;

  private:
   virtual void lock() = 0;
   virtual void unlock() = 0;

   Mutex(const Mutex&) = delete;
   Mutex &operator=(const Mutex&) = delete;

  public:
   Mutex() {}
   virtual ~Mutex() {}

   /////////////////////////////////////////////////////////////////////////
   void run_while_locked(std::function<void()> f)
   /////////////////////////////////////////////////////////////////////////
   {
    lock();

    std::exception_ptr exception;

    try
    {
     f();
    }
    catch (...)
    {
     exception = std::current_exception();
    }

    try
    {
     unlock();
    }
    catch (...)
    {
     if (!exception) // ??? maybe create a combined exception if both failed
      throw;
    }

    if (exception)
     std::rethrow_exception(exception);
   }
 };
}

#endif
