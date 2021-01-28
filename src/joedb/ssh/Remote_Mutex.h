#ifndef joedb_ssh_Remote_Mutex_declared
#define joedb_ssh_Remote_Mutex_declared

#include "joedb/concurrency/Mutex.h"

#include <string>

namespace joedb
{
 namespace ssh
 {
  class Thread_Safe_Session;

  ///////////////////////////////////////////////////////////////////////////
  class Remote_Mutex: public Mutex
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

    void lock() override;
    void unlock() override;
  };
 }
}

#endif
