#ifndef joedb_SSH_Connection_declared
#define joedb_SSH_Connection_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/Exception.h"

#ifdef JOEDB_HAS_SSH

#include "joedb/ssh/wrappers.h"
#include "joedb/ssh/Thread_Safe_Session.h"
#include "joedb/ssh/Keepalive_Thread.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class SSH_Connection: public Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class SSH_Robust_Connection;

  private:
   const std::string remote_file_name;
   const bool trace;

   const std::string mutex_file_name;
   const std::string full_remote_name;

   ssh::Thread_Safe_Session thread_safe_ssh_session;
   ssh::Keepalive_Thread keepalive_thread;

   void lock();
   void unlock();
   int64_t raw_pull(Writable_Journal &client_journal);
   void raw_push(Readonly_Journal &client_journal, int64_t server_position);

   int64_t pull(Writable_Journal &client_journal) override
   {
    lock();
    const int64_t result = raw_pull(client_journal);
    unlock();
    return result;
   }

   int64_t lock_pull(Writable_Journal &client_journal) override
   {
    lock();
    return raw_pull(client_journal);
   }

   void push_unlock
   (
    Readonly_Journal &client_journal,
    int64_t server_position
   ) override
   {
    raw_push(client_journal, server_position);
    unlock();
   }

  public:
   SSH_Connection
   (
    std::string user,
    std::string host,
    int port,
    std::string remote_file_name,
    bool trace,
    int ssh_log_level
   );
 };
}

#else

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class SSH_Connection: public Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class SSH_Robust_Connection;

  private:
   virtual int64_t pull(Writable_Journal &client_journal) {return 0;}

   virtual int64_t lock_pull(Writable_Journal &client_journal) {return 0;}

   virtual void push_unlock
   (
    Readonly_Journal &client_journal,
    int64_t server_position
   )
   {
   }

  public:
   SSH_Connection
   (
    std::string user,
    std::string host,
    int port,
    std::string remote_file_name,
    bool trace,
    int ssh_log_level
   )
   {
    throw Exception("SSH connection is not supported");
   }
 };
}

#endif
#endif
