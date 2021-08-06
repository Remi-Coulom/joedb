#ifndef joedb_Client_declared
#define joedb_Client_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/Exception.h"

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

   void check_position()
   {
    if (journal.get_position() > server_position)
     throw Exception("Ahead of server");
   }

   void lock_pull()
   {
    check_position();
    server_position = connection.lock_pull(journal);
    journal.play_until_checkpoint(writable);
   }

   void push_unlock()
   {
    journal.checkpoint(0);
    connection.push_unlock(journal, server_position);
    server_position = journal.get_checkpoint_position();
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
    server_position = journal.get_position();
   }

   int64_t pull()
   {
    check_position();
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
     throw;
    }

    push_unlock();
   }
 };
}

#endif
