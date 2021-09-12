#ifndef joedb_Client_declared
#define joedb_Client_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/Exception.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Client
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Connection &connection;
   Writable_Journal &journal;
   Writable &writable;
   int64_t server_checkpoint;

   void throw_if_pull_when_ahead()
   {
    if (journal.get_position() > server_checkpoint)
     throw Exception("can't pull: client is ahead of server");
   }

  public:
   //////////////////////////////////////////////////////////////////////////
   Client
   //////////////////////////////////////////////////////////////////////////
   (
    Connection &connection,
    Writable_Journal &journal,
    Writable &writable
   ):
    connection(connection),
    journal(journal),
    writable(writable)
   {
    server_checkpoint = connection.handshake(journal);
    const int64_t client_checkpoint = journal.get_checkpoint_position();

    if
    (
     !connection.check_matching_content
     (
      journal,
      std::min(server_checkpoint, client_checkpoint)
     )
    )
    {
     throw Exception("Client data does not match the server");
    }

    if (client_checkpoint > server_checkpoint)
     push_unlock(); // TODO: optional

    journal.play_until_checkpoint(writable);
   }

   //////////////////////////////////////////////////////////////////////////
   void push_unlock()
   //////////////////////////////////////////////////////////////////////////
   {
    connection.push_unlock(journal, server_checkpoint);
    server_checkpoint = journal.get_checkpoint_position();
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t pull()
   //////////////////////////////////////////////////////////////////////////
   {
    throw_if_pull_when_ahead();
    server_checkpoint = connection.pull(journal);
    journal.play_until_checkpoint(writable);
    return server_checkpoint;
   }

   //////////////////////////////////////////////////////////////////////////
   template<typename F> void transaction(F transaction)
   //////////////////////////////////////////////////////////////////////////
   {
    throw_if_pull_when_ahead();
    server_checkpoint = connection.lock_pull(journal);

    try
    {
     journal.play_until_checkpoint(writable);
     transaction();
     journal.checkpoint(Commit_Level::no_commit);
    }
    catch (...)
    {
     connection.unlock();
     throw;
    }

    push_unlock();
   }
 };
}

#endif
