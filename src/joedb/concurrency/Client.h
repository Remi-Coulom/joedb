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
   Client_Data &data;
   Connection &connection;
   int64_t server_checkpoint;

  public:
   //////////////////////////////////////////////////////////////////////////
   Client
   //////////////////////////////////////////////////////////////////////////
   (
    Client_Data &data,
    Connection &connection
   ):
    data(data),
    connection(connection),
    server_checkpoint(connection.handshake())
   {
    data.update();
   }

   Client_Data &get_data() {return data;}

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
    data.update();
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
     data.update();
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
