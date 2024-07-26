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
    const int64_t client_position = client_journal.get_position();

    client_journal.replay_log(writable);
    client_journal.set_position(client_position);

    return client_journal.get_checkpoint_position();
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t push
   //////////////////////////////////////////////////////////////////////////
   (
    Readonly_Journal &client_journal,
    int64_t server_position,
    bool unlock_after
   ) override
   {
    const int64_t client_position = client_journal.get_position();

    client_journal.set_position(server_position);
    client_journal.play_until_checkpoint(writable);
    client_journal.set_position(client_position);

    return client_journal.get_checkpoint_position();
   }
 };
}

#endif
