#ifndef joedb_Mutex_declared
#define joedb_Mutex_declared

#include <exception>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Mutex
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   virtual void lock() = 0;
   virtual void unlock() = 0;
   virtual ~Mutex() {}
 };

 ///////////////////////////////////////////////////////////////////////////
 class Mutex_Lock
 ///////////////////////////////////////////////////////////////////////////
 {
  private:
   Mutex &mutex;

  public:
   Mutex_Lock(Mutex &mutex): mutex(mutex)
   {
    mutex.lock();
   }

   ~Mutex_Lock() noexcept(false)
   {
    try
    {
     mutex.unlock();
    }
    catch (...)
    {
     if (!std::uncaught_exception())
      throw;
    }
   }
 };
}

#endif
