#ifndef joedb_ssh_Remote_Mutex_declared
#define joedb_ssh_Remote_Mutex_declared

#include <string>

namespace joedb
{
 namespace ssh
 {
  class Thread_Safe_Session;

  ///////////////////////////////////////////////////////////////////////////
  class Remote_Mutex
  ///////////////////////////////////////////////////////////////////////////
  {
   public:
    Thread_Safe_Session &session;
    const std::string remote_file_name;
    const bool trace;

    const std::string mutex_file_name;
    const std::string full_remote_name;

   public:
    Remote_Mutex
    (
     Thread_Safe_Session &session,
     std::string remote_file_name,
     bool trace
    );

    void lock();
    void unlock();
  };

  ///////////////////////////////////////////////////////////////////////////
  class Remote_Lock
  ///////////////////////////////////////////////////////////////////////////
  {
   private:
    Remote_Mutex &mutex;

   public:
    Remote_Lock(Remote_Mutex &mutex): mutex(mutex)
    {
     mutex.lock();
    }

    ~Remote_Lock()
    {
     mutex.unlock();
    }
  };
 }
}

#endif
