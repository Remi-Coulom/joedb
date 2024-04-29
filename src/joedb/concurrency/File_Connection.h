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
    client_journal.lock();
    server_journal.lock_pull();
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
    int64_t result = client_journal.pull_from(server_journal);
    client_journal.unlock();
    return result;
   }

   //////////////////////////////////////////////////////////////////////////
   virtual int64_t lock_pull(Writable_Journal &client_journal)
   //////////////////////////////////////////////////////////////////////////
   {
    client_journal.lock_pull();
    server_journal.lock_pull();
    return client_journal.pull_from(server_journal);
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
    if (!server_journal.is_locked())
     server_journal.lock_pull();

    if (server_checkpoint != server_journal.get_checkpoint_position())
     throw Exception("Conflict: push failed");

    server_journal.pull_from(client_journal);

    if (unlock_after)
     unlock(client_journal);
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
