#ifndef joedb_Connection_declared
#define joedb_Connection_declared

#include "joedb/journal/Writable_Journal.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Readonly_Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   static void content_mismatch();

  public:
   virtual int64_t handshake(Readonly_Journal &client_journal)
   {
    return client_journal.get_checkpoint_position();
   }

   virtual int64_t pull(Writable_Journal &client_journal)
   {
    client_journal.pull();
    return client_journal.get_checkpoint_position();
   }

   virtual ~Readonly_Connection();
 };

 ////////////////////////////////////////////////////////////////////////////
 class Connection: public Readonly_Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   virtual int64_t lock_pull(Writable_Journal &client_journal)
   {
    client_journal.lock_pull();
    return client_journal.get_checkpoint_position();
   }

   virtual int64_t push
   (
    Readonly_Journal &client_journal,
    int64_t server_checkpoint,
    bool unlock_after
   )
   {
    if (unlock_after)
     client_journal.unlock();
    return client_journal.get_checkpoint_position();
   }

   virtual void unlock(Readonly_Journal &client_journal)
   {
    client_journal.unlock();
   }
 };
}

#endif
