#include "joedb/ssh/Keepalive_Thread.h"
#include "joedb/ssh/Thread_Safe_Session.h"

namespace joedb
{
 namespace ssh
 {
  ///////////////////////////////////////////////////////////////////////////
  void Keepalive_Thread::keepalive()
  ///////////////////////////////////////////////////////////////////////////
  {
   ssh::Session_Lock lock(session);

   while (true)
   {
    condition.wait_for(lock, std::chrono::seconds(interval));

    if (must_stop)
     break;
    else
     ssh_send_ignore(lock.get_ssh_session(), "keepalive");
   }
  }

  ///////////////////////////////////////////////////////////////////////////
  Keepalive_Thread::Keepalive_Thread(Thread_Safe_Session &session):
  ///////////////////////////////////////////////////////////////////////////
   session(session),
   must_stop(false),
   thread([this](){keepalive();})
  {
  }

  ///////////////////////////////////////////////////////////////////////////
  Keepalive_Thread::~Keepalive_Thread()
  ///////////////////////////////////////////////////////////////////////////
  {
   {
    ssh::Session_Lock lock(session);
    must_stop = true;
    condition.notify_one();
   }
   thread.join();
  }
 }
}
