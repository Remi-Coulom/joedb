#ifndef joedb_ssh_Thread_Safe_Session_declared
#define joedb_ssh_Thread_Safe_Session_declared

#include "joedb/ssh/Session.h"
#include "joedb/ssh/SFTP.h"

#include <mutex>

namespace joedb
{
 namespace ssh
 {
  class Thread_Safe_Session;

  ///////////////////////////////////////////////////////////////////////////
  class Session_Lock
  ///////////////////////////////////////////////////////////////////////////
  {
   private:
    std::unique_lock<std::mutex> lock;
    Thread_Safe_Session &thread_safe_session;

   public:
    Session_Lock(Thread_Safe_Session &thread_safe_session);
    operator std::unique_lock<std::mutex> &() {return lock;}

    ssh_session get_ssh_session() const;
    sftp_session get_sftp_session() const;
  };

  ///////////////////////////////////////////////////////////////////////////
  class Thread_Safe_Session
  ///////////////////////////////////////////////////////////////////////////
  {
   friend class Session_Lock;

   private:
    std::mutex mutex;
    ssh::Session session;
    ssh::SFTP sftp;

   public:
    Thread_Safe_Session
    (
     std::string user,
     std::string host,
     int port,
     int ssh_log_level
    ):
     session(user, host, port, ssh_log_level),
     sftp(session)
    {
    }

    Session_Lock get_lock()
    {
     return Session_Lock(*this);
    }

    const std::string &get_user() {return session.get_user();}
    const std::string &get_host() {return session.get_host();}
  };

  ///////////////////////////////////////////////////////////////////////////
  inline Session_Lock::Session_Lock
  ///////////////////////////////////////////////////////////////////////////
  (
   Thread_Safe_Session &thread_safe_session
  ):
   lock(thread_safe_session.mutex),
   thread_safe_session(thread_safe_session)
  {
  }

  ///////////////////////////////////////////////////////////////////////////
  inline ssh_session Session_Lock::get_ssh_session() const
  ///////////////////////////////////////////////////////////////////////////
  {
   return thread_safe_session.session.get();
  }

  ///////////////////////////////////////////////////////////////////////////
  inline sftp_session Session_Lock::get_sftp_session() const
  ///////////////////////////////////////////////////////////////////////////
  {
   return thread_safe_session.sftp.get();
  }
 }
}

#endif
