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
   void do_push(bool unlock_after)
   {
    connection.push(data.get_journal(), server_checkpoint, unlock_after);
    server_checkpoint = data.get_journal().get_checkpoint_position();
   }

   void push(bool unlock_after)
   {
    const int64_t difference = get_checkpoint_difference();

    if (difference < 0)
     throw Exception("can't push: server is ahead of client");

    if (difference > 0)
     do_push(unlock_after);
    else if (unlock_after)
     connection.unlock();
   }

   void throw_if_pull_when_ahead()
   {
    if (data.get_journal().get_position() > server_checkpoint)
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
    server_checkpoint(connection.handshake(data.get_journal().is_shared()))
   {
    if (data.get_journal().is_shared())
     connection.unlock();

    {
     const int64_t client_checkpoint =
      data.get_journal().get_checkpoint_position();

     if
     (
      !connection.check_matching_content
      (
       data.get_journal(),
       std::min(server_checkpoint, client_checkpoint)
      )
     )
     {
      throw Exception("Client data does not match the server");
     }
    }

    data.update();
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t get_checkpoint_difference() const
   //////////////////////////////////////////////////////////////////////////
   {
    return data.get_journal().get_checkpoint_position() - server_checkpoint;
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

    if (data.get_journal().is_shared())
     server_checkpoint = connection.lock_pull_unlock(data.get_journal());
    else
     server_checkpoint = connection.pull(data.get_journal());

    data.update();
    return server_checkpoint;
   }

   //////////////////////////////////////////////////////////////////////////
   template<typename F> void transaction(F transaction)
   //////////////////////////////////////////////////////////////////////////
   {
    throw_if_pull_when_ahead();
    server_checkpoint = connection.lock_pull(data.get_journal());

    try
    {
     data.update();
     transaction();
     data.get_journal().checkpoint(Commit_Level::no_commit);
    }
    catch (...)
    {
     connection.unlock();
     data.get_journal().flush();
     throw;
    }

    push_unlock();
   }

   virtual ~Client() = default;
 };
}

#endif
