#ifndef joedb_Test_Sequence_declared
#define joedb_Test_Sequence_declared

#include <mutex>
#include <condition_variable>

//#define JOEDB_SEQUENCE_LOG
#ifdef JOEDB_SEQUENCE_LOG
#include <iostream>
#endif

namespace joedb
{
 /////////////////////////////////////////////////////////////////////////////
 class Test_Sequence
 /////////////////////////////////////////////////////////////////////////////
 {
  private:
   std::mutex mutex;
   std::condition_variable condition;
   int n = 0;

   void log()
   {
#ifdef JOEDB_SEQUENCE_LOG
    std::cerr << "sent: " << n << '\n';
#endif
   }

  public:
   void send(int new_n)
   {
    {
     std::unique_lock<std::mutex> lock(mutex);
     n = new_n;
     log();
    }
    condition.notify_all();
   }

   void increment()
   {
    {
     std::unique_lock<std::mutex> lock(mutex);
     n++;
     log();
    }
    condition.notify_all();
   }

   int get()
   {
    std::unique_lock<std::mutex> lock(mutex);
    return n;
   }

   void wait_for(int awaited_n)
   {
    std::unique_lock<std::mutex> lock(mutex);
    while (awaited_n > n)
     condition.wait(lock);
#ifdef JOEDB_SEQUENCE_LOG
    std::cerr << "waited for " << n << '\n';
#endif
   }
 };
}

#endif
