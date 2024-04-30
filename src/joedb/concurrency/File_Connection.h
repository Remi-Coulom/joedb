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

   //////////////////////////////////////////////////////////////////////////
   int64_t handshake(Readonly_Journal &client_journal) final
   //////////////////////////////////////////////////////////////////////////
   {
    const int64_t min = std::min
    (
     server_journal.get_checkpoint_position(),
     client_journal.get_checkpoint_position()
    );

    if (client_journal.get_hash(min) != server_journal.get_hash(min))
     content_mismatch();

    return server_journal.get_checkpoint_position();
   }

   //////////////////////////////////////////////////////////////////////////
   void unlock(Readonly_Journal &client_journal) final
   //////////////////////////////////////////////////////////////////////////
   {
    server_journal.unlock();
    client_journal.unlock();
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t pull(Writable_Journal &client_journal) final
   //////////////////////////////////////////////////////////////////////////
   {
    client_journal.lock_pull();
    server_journal.pull();

    client_journal.pull_from(server_journal);
    client_journal.unlock();

    return server_journal.get_checkpoint_position();
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t lock_pull(Writable_Journal &client_journal) final
   //////////////////////////////////////////////////////////////////////////
   {
    client_journal.lock_pull();
    server_journal.lock_pull();

    client_journal.pull_from(server_journal);

    return server_journal.get_checkpoint_position();
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t push
   //////////////////////////////////////////////////////////////////////////
   (
    Readonly_Journal &client_journal,
    const int64_t server_checkpoint,
    bool unlock_after
   ) final
   {
    if (!server_journal.is_locked())
     server_journal.lock_pull();
    client_journal.pull();

    server_journal.pull_from(client_journal);

    if (unlock_after)
     unlock(client_journal);

    return server_journal.get_checkpoint_position();
   }

  public:
   //////////////////////////////////////////////////////////////////////////
   File_Connection
   //////////////////////////////////////////////////////////////////////////
   (
    Generic_File &server_file,
    Readonly_Journal::Check check = Readonly_Journal::Check::all,
    Commit_Level commit_level = Commit_Level::no_commit
   ):
    server_journal(server_file, check, commit_level)
   {
   }
 };
}

#endif
