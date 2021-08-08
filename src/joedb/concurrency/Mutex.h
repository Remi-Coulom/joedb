#ifndef joedb_Mutex_declared
#define joedb_Mutex_declared

#include "joedb/Posthumous_Thrower.h"

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

  public:
   virtual ~Mutex() {}
 };

 ///////////////////////////////////////////////////////////////////////////
 class Mutex_Lock: public Posthumous_Thrower
 ///////////////////////////////////////////////////////////////////////////
 {
  private:
   Mutex &mutex;

  public:
   Mutex_Lock(Mutex &mutex): mutex(mutex)
   {
    mutex.lock();
   }

   ~Mutex_Lock()
   {
    try
    {
     mutex.unlock();
    }
    catch (...)
    {
     postpone_exception();
    }
   }
 };
}

#endif
