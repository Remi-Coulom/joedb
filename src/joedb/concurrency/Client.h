#ifndef joedb_Client_declared
#define joedb_Client_declared

#include "joedb/concurrency/Connection.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 template<typename Client_Data> class Client
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   Connection &connection;
   int64_t server_checkpoint;
   Client_Data data;

   void push_unlock()
   {
    connection.push_unlock(data.get_journal(), server_checkpoint);
    server_checkpoint = data.get_journal().get_checkpoint_position();
   }

   void throw_if_pull_when_ahead()
   {
    if (data.get_journal().get_position() > server_checkpoint)
     throw Exception("can't pull: client is ahead of server");
   }

  public:
   //////////////////////////////////////////////////////////////////////////
   Client
   //////////////////////////////////////////////////////////////////////////
   (
    Connection &connection,
    Generic_File &file
   ):
    connection(connection),
    server_checkpoint(connection.handshake()),
    data((connection.lock(), connection), file)
   {
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
   void push()
   //////////////////////////////////////////////////////////////////////////
   {
    if (get_checkpoint_difference() > 0)
     push_unlock();
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t pull()
   //////////////////////////////////////////////////////////////////////////
   {
    throw_if_pull_when_ahead();
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
     data.get_journal().flush();
     connection.unlock();
     throw;
    }

    push_unlock();
   }
 };
}

#endif
