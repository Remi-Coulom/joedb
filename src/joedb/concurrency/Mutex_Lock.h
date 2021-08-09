#ifndef joedb_Mutex_Lock_declared
#define joedb_Mutex_Lock_declared

#include "joedb/concurrency/Mutex.h"
#include "joedb/Posthumous_Thrower.h"

namespace joedb
{
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
