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

   Mutex(const Mutex&) = delete;
   Mutex &operator=(const Mutex&) = delete;

  public:
   Mutex() {}
   virtual ~Mutex() {}
 };

 ///////////////////////////////////////////////////////////////////////////
 class Mutex_Lock: public Posthumous_Thrower
 ///////////////////////////////////////////////////////////////////////////
 {
  private:
   Mutex &mutex;

   Mutex_Lock(const Mutex_Lock&) = delete;
   Mutex_Lock &operator=(const Mutex_Lock&) = delete;

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
     postpone_exception("could not unlock mutex");
    }
   }
 };
}

#endif
