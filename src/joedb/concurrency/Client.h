#ifndef joedb_Client_declared
#define joedb_Client_declared

#include "joedb/concurrency/Push_Only_Client.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Client: public Push_Only_Client
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Writable &writable;

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
    Push_Only_Client(connection, journal),
    writable(writable)
   {
    journal.play_until_checkpoint(writable);
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
     journal.flush();
     connection.unlock();
     throw;
    }

    push_unlock();
   }
 };
}

#endif
