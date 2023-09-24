#ifndef joedb_Readonly_File_Connection_declared
#define joedb_Readonly_File_Connection_declared

#include "joedb/concurrency/Connection.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Readonly_File_Connection: public Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Readonly_Journal server_journal;

   //////////////////////////////////////////////////////////////////////////
   int64_t handshake(Readonly_Journal &client_journal) final
   //////////////////////////////////////////////////////////////////////////
   {
    check_not_shared(client_journal);

    const int64_t server_position = server_journal.get_checkpoint_position();
    const int64_t client_position = client_journal.get_checkpoint_position();

    const int64_t min = std::min(server_position, client_position);

    if (client_journal.get_hash(min) != server_journal.get_hash(min))
     content_mismatch();

    return server_position;
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t pull(Writable_Journal &client_journal) final
   //////////////////////////////////////////////////////////////////////////
   {
    server_journal.refresh_checkpoint();

    const int64_t client_checkpoint=client_journal.get_checkpoint_position();
    const int64_t server_checkpoint=server_journal.get_checkpoint_position();

    if (client_checkpoint < server_checkpoint)
    {
     client_journal.append_raw_tail
     (
      server_journal.get_raw_tail(client_checkpoint)
     );
    }

    return server_checkpoint;
   }

   bool is_readonly() const final {return true;}

  public:
   //////////////////////////////////////////////////////////////////////////
   Readonly_File_Connection(Generic_File &server_file):
   //////////////////////////////////////////////////////////////////////////
    server_journal(server_file)
   {
   }
 };
}

#endif
