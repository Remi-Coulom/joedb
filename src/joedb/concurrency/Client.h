#ifndef joedb_Client_declared
#define joedb_Client_declared

#include "joedb/concurrency/Connection.h"

#include <functional>

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

   int64_t server_position;

   void lock_pull()
   {
    server_position = connection.lock_pull(journal);
    journal.play_until_checkpoint(writable);
   }

   void push_unlock()
   {
    journal.checkpoint(0);
    connection.push_unlock(journal, server_position);
   }

  public:
   Client
   (
    Connection &connection,
    Writable_Journal &journal,
    Writable &writable
   ):
    connection(connection),
    journal(journal),
    writable(writable)
   {
    journal.play_until_checkpoint(writable);
   }

   int64_t pull()
   {
    server_position = connection.pull(journal);
    journal.play_until_checkpoint(writable);
    return server_position;
   }

   void write_transaction(std::function<void()> transaction)
   {
    lock_pull();

    try
    {
     transaction();
    }
    catch (...)
    {
     connection.unlock();
     journal.checkpoint(0);
#if 0 // take care of this later
     if (journal.get_checkpoint_position() != server_position)
      // kill connection
      ;
#endif
     throw;
    }

    push_unlock();
   }
 };
}

#endif
