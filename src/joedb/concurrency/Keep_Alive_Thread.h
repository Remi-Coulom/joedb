#ifndef joedb_Keep_Alive_Thread_declared
#define joedb_Keep_Alive_Thread_declared

#include "joedb/concurrency/Channel.h"
#include "joedb/Thread_Safe.h"

#include <thread>
#include <condition_variable>

namespace joedb
{
 class Ping_Client
 {
  public:
   virtual Thread_Safe<Channel&> &get_channel() = 0;
   virtual void ping(Lock<Channel&> &lock) = 0;
   virtual ~Ping_Client() = default;
 };

 class Keep_Alive_Thread
 {
  private:
   Ping_Client &client;
   const std::chrono::milliseconds interval;

   bool thread_must_stop;
   bool stopped;
   std::thread thread;
   std::condition_variable condition;

   void keep_alive()
   {
    try
    {
     Lock<Channel&> lock(client.get_channel());

     while (!thread_must_stop)
     {
      condition.wait_for(lock, interval);

      if (thread_must_stop)
       break;

      client.ping(lock);
     }
    }
    catch(...)
    {
    }
   }

  public:
   Keep_Alive_Thread(Ping_Client &client, std::chrono::milliseconds interval):
    client(client),
    interval(interval),
    thread_must_stop(false),
    stopped(true)
   {
    if (interval.count() > 0)
    {
     thread = std::thread([this](){keep_alive();});
     stopped = false;
    }
   }

   void ping()
   {
    Lock<Channel&> lock(client.get_channel());
    client.ping(lock);
   }

   void stop()
   {
    if (!stopped)
    {
     {
      Lock<Channel&> lock(client.get_channel());
      thread_must_stop = true;
     }
     condition.notify_one();
     if (thread.joinable())
      thread.join();
     stopped = true;
    }
   }

   ~Keep_Alive_Thread()
   {
    if (!stopped)
     try {stop();} catch (...) {}
   }
 };
}

#endif
