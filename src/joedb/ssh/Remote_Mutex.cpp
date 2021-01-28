#include "joedb/ssh/Remote_Mutex.h"
#include "joedb/ssh/Thread_Safe_Session.h"

#include <fcntl.h>
#include <thread>
#include <chrono>
#include <iostream>

namespace joedb
{
 namespace ssh
 {
  ///////////////////////////////////////////////////////////////////////////
  Remote_Mutex::Remote_Mutex
  ///////////////////////////////////////////////////////////////////////////
  (
   Thread_Safe_Session &session,
   std::string remote_file_name,
   bool trace
  ):
   session(session),
   remote_file_name(remote_file_name),
   trace(trace),
   mutex_file_name(remote_file_name + ".mutex"),
   full_remote_name
   (
    session.get_user() + "@" + session.get_host() + ":" + remote_file_name
   )
  {
  }

  ///////////////////////////////////////////////////////////////////////////
  void Remote_Mutex::lock()
  ///////////////////////////////////////////////////////////////////////////
  {
   ssh::Session_Lock lock(session);

   if (trace)
    std::cerr << full_remote_name << ": lock()... ";

   while (true)
   {
    sftp_file file = sftp_open
    (
     lock.get_sftp_session(),
     mutex_file_name.c_str(),
     O_CREAT | O_EXCL,
     S_IRUSR
    );

    if (file)
    {
     sftp_close(file);
     break;
    }
    else
    {
     if (trace)
      std::cerr << '.';
     std::this_thread::sleep_for(std::chrono::seconds(1));
    }
   }

   if (trace)
    std::cerr << "done.\n";
  }

  ///////////////////////////////////////////////////////////////////////////
  void Remote_Mutex::unlock()
  ///////////////////////////////////////////////////////////////////////////
  {
   ssh::Session_Lock lock(session);

   if (trace)
    std::cerr << full_remote_name << ": unlock()\n";

   if (sftp_unlink(lock.get_sftp_session(), mutex_file_name.c_str()) < 0)
    throw Exception("Error removing remote mutex");
  }
 }
}
