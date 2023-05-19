#ifndef joedb_Client_declared
#define joedb_Client_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/concurrency/Client_Data.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Client
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   void push(bool unlock_after)
   {
    const int64_t difference = get_checkpoint_difference();

    if (difference < 0)
     throw Exception("can't push: server is ahead of client");

    if (difference > 0)
    {
     connection.push(server_checkpoint, unlock_after);
     server_checkpoint = connection.client_journal.get_checkpoint_position();
    }
    else if (unlock_after)
     connection.unlock();
   }

   void throw_if_pull_when_ahead()
   {
    if (connection.client_journal.get_position() > server_checkpoint)
     throw Exception("can't pull: client is ahead of server");
   }

  protected:
   Connection &connection;
   Client_Data &data;
   int64_t server_checkpoint;

  public:
   //////////////////////////////////////////////////////////////////////////
   Client
   //////////////////////////////////////////////////////////////////////////
   (
    Connection &connection,
    Client_Data &data
   ):
    connection(connection),
    data(data),
    server_checkpoint(connection.handshake())
   {
    if (!connection.check_matching_content(server_checkpoint))
     throw Exception("Client data does not match the server");

    data.update(connection.client_journal);
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t get_checkpoint_difference() const
   //////////////////////////////////////////////////////////////////////////
   {
    return connection.client_journal.get_checkpoint_position() - server_checkpoint;
   }

   //////////////////////////////////////////////////////////////////////////
   void locked_push()
   //////////////////////////////////////////////////////////////////////////
   {
    push(false);
   }

   //////////////////////////////////////////////////////////////////////////
   void push_unlock()
   //////////////////////////////////////////////////////////////////////////
   {
    push(true);
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t pull()
   //////////////////////////////////////////////////////////////////////////
   {
    throw_if_pull_when_ahead();
    server_checkpoint = connection.pull();
    data.update(connection.client_journal);
    return server_checkpoint;
   }

   //////////////////////////////////////////////////////////////////////////
   template<typename F> void transaction(F transaction)
   //////////////////////////////////////////////////////////////////////////
   {
    throw_if_pull_when_ahead();
    server_checkpoint = connection.lock_pull();

    try
    {
     data.update(connection.client_journal);
     transaction();
     connection.client_journal.checkpoint(Commit_Level::no_commit);
    }
    catch (...)
    {
     connection.unlock();
     throw;
    }

    push_unlock();
   }

   virtual ~Client() = default;
 };
}

#endif
