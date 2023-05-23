#ifndef joedb_Local_Connection_declared
#define joedb_Local_Connection_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/journal/File.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Local_Connection: public Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   int64_t handshake(Readonly_Journal &client_journal) final
   {
    check_shared(client_journal);
    return client_journal.get_checkpoint_position();
   }

   void lock(Readonly_Journal &client_journal) final
   {
    client_journal.exclusive_lock();
   }

   void unlock(Readonly_Journal &client_journal) final
   {
    client_journal.unlock();
   }

   int64_t pull(Writable_Journal &client_journal) final
   {
    client_journal.refresh_checkpoint();
    return client_journal.get_checkpoint_position();
   }

   int64_t lock_pull(Writable_Journal &client_journal) final
   {
    client_journal.exclusive_lock();
    return pull(client_journal);
   }

   void push
   (
    Readonly_Journal &client_journal,
    int64_t server_position,
    bool unlock_after
   ) final
   {
    if (unlock_after)
     unlock(client_journal);
   }
 };
}

#endif
