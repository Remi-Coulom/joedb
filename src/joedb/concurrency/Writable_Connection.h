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
   int64_t push_until
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

    const int64_t end_position = std::min
    (
     journal.get_checkpoint_position(),
     until_position
    );

    journal.set_position(from_position);
    journal.play_until(writable, end_position);
    writable.default_checkpoint();

    return client_journal.get_checkpoint_position();
   }
 };
}

#endif
