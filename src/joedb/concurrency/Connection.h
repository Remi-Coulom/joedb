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
   void content_mismatch()
   {
    throw Exception("Client data does not match the server");
   }

  public:
   virtual int64_t handshake(Readonly_Journal &client_journal)
   {
    return client_journal.get_checkpoint_position();
   }

   virtual void lock(Readonly_Journal &client_journal) {}
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
   )
   {
   }

   virtual ~Connection() = default;
 };
}

#endif
