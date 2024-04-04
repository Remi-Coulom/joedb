#ifndef joedb_File_Connection_declared
#define joedb_File_Connection_declared

#include "joedb/concurrency/Connection.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class File_Connection: public Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Writable_Journal server_journal;
   bool locked;

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
   void lock(Readonly_Journal &client_journal) final
   //////////////////////////////////////////////////////////////////////////
   {
    if (locked)
     throw Exception("Deadlock detected");
    locked = true;
   }

   //////////////////////////////////////////////////////////////////////////
   void unlock(Readonly_Journal &client_journal) final
   //////////////////////////////////////////////////////////////////////////
   {
    locked = false;
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t pull(Writable_Journal &client_journal) final
   //////////////////////////////////////////////////////////////////////////
   {
    return client_journal.pull(server_journal);
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

    server_journal.pull(client_journal);

    if (unlock_after)
     unlock(client_journal);
   }

  public:
   //////////////////////////////////////////////////////////////////////////
   File_Connection
   //////////////////////////////////////////////////////////////////////////
   (
    Generic_File &server_file,
    Commit_Level commit_level = Commit_Level::no_commit
   ):
    server_journal(server_file, commit_level),
    locked(false)
   {
    check_not_shared(server_journal);
   }
 };
}

#endif
