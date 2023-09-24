#ifndef joedb_Connection_declared
#define joedb_Connection_declared

#include "joedb/journal/Writable_Journal.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   static void content_mismatch();
   static void check_shared(Readonly_Journal &client_journal);
   static void check_not_shared(Readonly_Journal &client_journal);

  public:
   virtual int64_t handshake(Readonly_Journal &client_journal)
   {
    check_not_shared(client_journal);
    return client_journal.get_checkpoint_position();
   }

   virtual void lock(Readonly_Journal &client_journal);
   virtual void unlock(Readonly_Journal &client_journal) {}

   virtual int64_t pull(Writable_Journal &client_journal)
   {
    return client_journal.get_checkpoint_position();
   }

   virtual int64_t lock_pull(Writable_Journal &client_journal)
   {
    lock(client_journal);
    return pull(client_journal);
   }

   virtual void push
   (
    Readonly_Journal &client_journal,
    int64_t server_checkpoint,
    bool unlock_after
   );

   virtual bool is_readonly() const {return false;}

   virtual ~Connection();
 };
}

#endif
