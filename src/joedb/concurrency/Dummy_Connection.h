#ifndef joedb_Dummy_Connection_declared
#define joedb_Dummy_Connection_declared

#include "joedb/concurrency/Connection.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Dummy_Connection: public Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   int64_t handshake(Readonly_Journal &client_journal) override
   {
    return client_journal.get_checkpoint_position();
   }

   bool check_matching_content
   (
    Readonly_Journal &client_journal,
    int64_t checkpoint
   ) override
   {
    return true;
   }

   void lock() override {}
   void unlock() override {}

   int64_t pull(Writable_Journal &client_journal) override
   {
    return client_journal.get_checkpoint_position();
   }

   int64_t lock_pull(Writable_Journal &client_journal) override
   {
    return client_journal.get_checkpoint_position();
   }

   int64_t lock_pull_unlock(Writable_Journal &client_journal) override
   {
    return client_journal.get_checkpoint_position();
   }

   void push
   (
    Readonly_Journal &client_journal,
    int64_t server_checkpoint,
    bool unlock_after
   ) override
   {
   }
 };
}

#endif
