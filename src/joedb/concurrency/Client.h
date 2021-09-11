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
   bool cancelled_transaction_data;

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
    writable(writable),
    cancelled_transaction_data(false)
   {
    const int64_t server_checkpoint = connection.handshake(journal);
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
    {
     connection.push_unlock(journal, server_checkpoint);
    }

    journal.rewind();
    journal.play_until_checkpoint(writable);
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t pull()
   //////////////////////////////////////////////////////////////////////////
   {
    const int64_t server_checkpoint = connection.pull(journal);
    journal.play_until_checkpoint(writable);
    return server_checkpoint;
   }

   //////////////////////////////////////////////////////////////////////////
   template<typename F> void transaction(F transaction)
   //////////////////////////////////////////////////////////////////////////
   {
    if (cancelled_transaction_data)
     throw Exception("Local journal contains cancelled transaction");

    const int64_t server_checkpoint = connection.lock_pull(journal);

    try
    {
     journal.play_until_checkpoint(writable);
     transaction();
     journal.checkpoint(Commit_Level::no_commit);
    }
    catch (...)
    {
     if (journal.get_position() > server_checkpoint)
      cancelled_transaction_data = true;
     connection.unlock();
     throw;
    }

    connection.push_unlock(journal, server_checkpoint);
   }
 };
}

#endif
