#ifndef joedb_Embedded_Connection_declared
#define joedb_Embedded_Connection_declared

#include "joedb/concurrency/Connection.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Embedded_Connection: public Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Writable_Journal server_journal;
   bool locked;

   //////////////////////////////////////////////////////////////////////////
   int64_t handshake() final
   //////////////////////////////////////////////////////////////////////////
   {
    return server_journal.get_checkpoint_position();
   }

   //////////////////////////////////////////////////////////////////////////
   bool check_matching_content
   //////////////////////////////////////////////////////////////////////////
   (
    Readonly_Journal &client_journal,
    int64_t checkpoint
   ) final
   {
    return
     client_journal.get_hash(checkpoint) ==
     server_journal.get_hash(checkpoint);
   }

   //////////////////////////////////////////////////////////////////////////
   void lock() final
   //////////////////////////////////////////////////////////////////////////
   {
    if (locked)
     throw Exception("Deadlock detected");
    locked = true;
   }

   //////////////////////////////////////////////////////////////////////////
   void unlock() final
   //////////////////////////////////////////////////////////////////////////
   {
    locked = false;
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t pull(Writable_Journal &client_journal) final
   //////////////////////////////////////////////////////////////////////////
   {
    const int64_t client_checkpoint=client_journal.get_checkpoint_position();
    const int64_t server_checkpoint=server_journal.get_checkpoint_position();

    if (client_checkpoint < server_checkpoint)
     client_journal.append_raw_tail
     (
      server_journal.get_raw_tail(client_checkpoint)
     );

    return server_checkpoint;
   }

   //////////////////////////////////////////////////////////////////////////
   void push
   //////////////////////////////////////////////////////////////////////////
   (
    Readonly_Journal &client_journal,
    const int64_t server_checkpoint,
    bool unlock_after
   ) final
   {
    if (server_checkpoint != server_journal.get_checkpoint_position())
     throw Exception("pushing from bad checkpoint");

    const int64_t client_checkpoint=client_journal.get_checkpoint_position();

    if (server_checkpoint < client_checkpoint)
     server_journal.append_raw_tail
     (
      client_journal.get_raw_tail(server_checkpoint)
     );

    if (unlock_after)
     unlock();
   }

  public:
   //////////////////////////////////////////////////////////////////////////
   Embedded_Connection(Generic_File &file):
   //////////////////////////////////////////////////////////////////////////
    server_journal(file),
    locked(false)
   {
   }
 };
}

#endif
