#ifndef joedb_Thread_Safe_declared
#define joedb_Thread_Safe_declared

#include <mutex>

namespace joedb
{
 template<typename T> class Lock;

 template<typename T> class Thread_Safe
 {
  friend class Lock<T>;

  private:
   std::mutex mutex;
   T t;

  public:
   template<class... Arguments> Thread_Safe(Arguments &&... a): t(a...)
   {
   }
 };

 template<typename T> class Lock
 {
  private:
   std::unique_lock<std::mutex> lock;
   T &t;

  public:
   Lock(Thread_Safe<T> &t): lock(t.mutex), t(t.t)
   {
   }

   operator std::unique_lock<std::mutex> &() {return lock;}

   auto *operator->() const {return &t;}
   auto *operator->() {return &t;}
   auto &operator*() const {return t;}
   auto &operator*() {return t;}
 };
}

#endif
