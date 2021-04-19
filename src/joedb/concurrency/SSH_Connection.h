#ifndef joedb_SSH_Connection_declared
#define joedb_SSH_Connection_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/ssh/Remote_Mutex.h"
#include "joedb/ssh/Keepalive_Thread.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class SSH_Connection: public Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   ssh::Remote_Mutex remote_mutex;
   ssh::Keepalive_Thread keepalive_thread;

   int64_t raw_pull(Writable_Journal &client_journal);
   void raw_push(Readonly_Journal &client_journal, int64_t server_position);

   int64_t pull(Writable_Journal &client_journal) override
   {
    Mutex_Lock lock(remote_mutex);
    const int64_t result = raw_pull(client_journal);
    return result;
   }

   int64_t lock_pull(Writable_Journal &client_journal) override
   {
    remote_mutex.lock();
    return raw_pull(client_journal);
   }

   void push_unlock
   (
    Readonly_Journal &client_journal,
    int64_t server_position
   ) override
   {
    raw_push(client_journal, server_position);
    remote_mutex.unlock();
   }

   void lock() override {remote_mutex.lock();}

   void unlock() override {remote_mutex.unlock();}

  public:
   SSH_Connection
   (
    ssh::Thread_Safe_Session &session,
    std::string remote_file_name,
    bool trace
   );

   ssh::Thread_Safe_Session &get_session() {return remote_mutex.session;}
 };
}

#endif
