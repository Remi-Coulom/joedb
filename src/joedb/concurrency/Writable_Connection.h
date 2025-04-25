#ifndef joedb_Writable_Connection_declared
#define joedb_Writable_Connection_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/journal/File_View.h"

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
    const Readonly_Journal &client_journal,
    bool contentcheck
   ) override
   {
    File_View file_view(client_journal.get_file());
    Readonly_Journal journal(file_view);
    journal.replay_log(writable);

    return client_journal.get_checkpoint_position();
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t push
   //////////////////////////////////////////////////////////////////////////
   (
    const Readonly_Journal &client_journal,
    int64_t from_position,
    int64_t until_position,
    bool unlock_after
   ) override
   {
    File_View file_view(client_journal.get_file());
    Readonly_Journal journal(file_view);
    journal.set_position(from_position);
    journal.play_until(writable, until_position);

    return client_journal.get_checkpoint_position();
   }
 };
}

#endif
