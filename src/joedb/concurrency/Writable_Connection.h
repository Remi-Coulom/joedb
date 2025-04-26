#ifndef joedb_Writable_Connection_declared
#define joedb_Writable_Connection_declared

#include "joedb/concurrency/Connection.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Writable_Connection: public Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Writable &writable;
   int64_t position;

  public:
   //////////////////////////////////////////////////////////////////////////
   Writable_Connection(Writable &writable): writable(writable)
   //////////////////////////////////////////////////////////////////////////
   {
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t handshake
   //////////////////////////////////////////////////////////////////////////
   (
    Readonly_Journal &client_journal,
    bool contentcheck
   ) override
   {
    client_journal.replay_log(writable);
    position = client_journal.get_checkpoint_position();
    return position;
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t push
   //////////////////////////////////////////////////////////////////////////
   (
    Readonly_Journal &client_journal,
    int64_t from,
    int64_t until,
    bool unlock_after
   ) override
   {
    if (from != position)
     throw Exception("pushing from wrong position");
    client_journal.rewind_until(from);
    client_journal.play_until(writable, until);
    position = until;
    return position;
   }
 };
}

#endif
