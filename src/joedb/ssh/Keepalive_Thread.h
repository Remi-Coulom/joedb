#ifndef joedb_ssh_Keepalive_Thread
#define joedb_ssh_Keepalive_Thread

#include <condition_variable>
#include <thread>

namespace joedb
{
 namespace ssh
 {
  class Thread_Safe_Session;

  ///////////////////////////////////////////////////////////////////////////
  class Keepalive_Thread
  ///////////////////////////////////////////////////////////////////////////
  {
   private:
    Thread_Safe_Session &session;

    enum {interval = 240};
    bool must_stop;
    std::condition_variable condition;
    std::thread thread;
    void keepalive();

   public:
    Keepalive_Thread(Thread_Safe_Session &session);
    ~Keepalive_Thread();
  };
 }
}

#endif
