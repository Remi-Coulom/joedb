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
   int64_t handshake() final
   {
    return client_journal.get_checkpoint_position();
   }

   void lock() final
   {
    client_journal.exclusive_lock();
   }

   void unlock() final
   {
    client_journal.unlock();
   }

   int64_t pull() final
   {
    client_journal.shared_transaction([this]()
    {
     client_journal.refresh_checkpoint();
    });

    return client_journal.get_checkpoint_position();
   }

   int64_t lock_pull() final
   {
    client_journal.exclusive_lock();
    client_journal.refresh_checkpoint();
    return client_journal.get_checkpoint_position();
   }

   void push
   (
    int64_t server_position,
    bool unlock_after
   ) final
   {
    if (unlock_after)
     unlock();
   }

   bool check_matching_content(int64_t server_checkpoint) final
   {
    return true;
   }

  public:
   Local_Connection(Writable_Journal &client_journal):
    Connection(client_journal)
   {
   }
 };
}

#endif
