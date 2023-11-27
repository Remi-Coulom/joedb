#ifndef joedb_Client_declared
#define joedb_Client_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/concurrency/Client_Data.h"
#include "joedb/Posthumous_Thrower.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Client
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Client_Lock;

  private:
   //////////////////////////////////////////////////////////////////////////
   void push(bool unlock_after)
   //////////////////////////////////////////////////////////////////////////
   {
    const int64_t difference = get_checkpoint_difference();

    if (difference < 0)
     throw Exception("can't push: server is ahead of client");

    if (difference > 0)
    {
     connection.push
     (
      data.get_readonly_journal(),
      server_checkpoint,
      unlock_after
     );

     server_checkpoint =
      data.get_readonly_journal().get_checkpoint_position();
    }
    else if (unlock_after)
     connection.unlock(data.get_readonly_journal());
   }

   //////////////////////////////////////////////////////////////////////////
   void throw_if_pull_when_ahead()
   //////////////////////////////////////////////////////////////////////////
   {
    if (data.get_readonly_journal().get_position() > server_checkpoint)
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
    server_checkpoint(connection.handshake(data.get_readonly_journal()))
   {
    data.update();
   }

   const Client_Data &get_data() const {return data;}
   const Readonly_Journal &get_journal() {return data.get_readonly_journal();}
   bool is_readonly() const {return data.is_readonly();}

   //////////////////////////////////////////////////////////////////////////
   int64_t get_checkpoint() const
   //////////////////////////////////////////////////////////////////////////
   {
    return data.get_readonly_journal().get_checkpoint_position();
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t get_checkpoint_difference() const
   //////////////////////////////////////////////////////////////////////////
   {
    return
     data.get_readonly_journal().get_checkpoint_position() -
     server_checkpoint;
   }

   //////////////////////////////////////////////////////////////////////////
   void refresh_data()
   //////////////////////////////////////////////////////////////////////////
   {
    data.refresh();
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
    server_checkpoint = connection.pull(data.get_writable_journal());
    data.update();
    return server_checkpoint;
   }

   //////////////////////////////////////////////////////////////////////////
   template<typename F> void transaction(F transaction)
   //////////////////////////////////////////////////////////////////////////
   {
    throw_if_pull_when_ahead();
    server_checkpoint = connection.lock_pull(data.get_writable_journal());

    try
    {
     data.update();
     transaction(data);
     data.get_writable_journal().default_checkpoint();
    }
    catch (...)
    {
     connection.unlock(data.get_writable_journal());
     data.get_writable_journal().flush();
     throw;
    }

    push_unlock();
   }

   virtual ~Client();
 };

 ////////////////////////////////////////////////////////////////////////////
 class Client_Lock: public Posthumous_Thrower
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Client &client;

  public:
   Client_Lock(Client &client): client(client)
   {
    client.throw_if_pull_when_ahead();
    client.server_checkpoint = client.connection.lock_pull
    (
     client.data.get_writable_journal()
    );
    client.data.update();
   }

   Writable_Journal &get_journal()
   {
    return client.data.get_writable_journal();
   }

   void push()
   {
    client.push(false);
   }

   ~Client_Lock()
   {
    try
    {
     client.push_unlock();
    }
    catch (...)
    {
     postpone_exception();
    }
   }
 };
}

#endif
