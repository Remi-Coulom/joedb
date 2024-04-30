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
   bool push(bool unlock_after)
   //////////////////////////////////////////////////////////////////////////
   {
    if (data.is_readonly())
     data.get_readonly_journal().pull();

    if (get_checkpoint_difference() > 0)
    {
     server_checkpoint = connection.push
     (
      data.get_readonly_journal(),
      server_checkpoint,
      unlock_after
     );

     return true;
    }

    if (unlock_after)
     connection.unlock(data.get_readonly_journal());

    return false;
   }

   //////////////////////////////////////////////////////////////////////////
   void cancel_transaction()
   //////////////////////////////////////////////////////////////////////////
   {
    connection.unlock(data.get_writable_journal());
    data.get_writable_journal().flush();
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
   }

   Client_Data &get_data() const {return data;}
   const Readonly_Journal &get_journal() {return data.get_readonly_journal();}
   bool is_readonly() const {return data.is_readonly();}

   //////////////////////////////////////////////////////////////////////////
   int64_t get_checkpoint() const
   //////////////////////////////////////////////////////////////////////////
   {
    return data.get_readonly_journal().get_checkpoint_position();
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t get_server_checkpoint() const
   //////////////////////////////////////////////////////////////////////////
   {
    return server_checkpoint;
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t get_checkpoint_difference() const
   //////////////////////////////////////////////////////////////////////////
   {
    return get_checkpoint() - server_checkpoint;
   }

   //////////////////////////////////////////////////////////////////////////
   bool push_unlock()
   //////////////////////////////////////////////////////////////////////////
   {
    return push(true);
   }

   //////////////////////////////////////////////////////////////////////////
   bool push_and_keep_locked()
   //////////////////////////////////////////////////////////////////////////
   {
    return push(false);
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t pull()
   //////////////////////////////////////////////////////////////////////////
   {
    if (data.is_readonly())
     data.get_readonly_journal().pull();
    else
    {
     throw_if_pull_when_ahead();
     server_checkpoint = connection.pull(data.get_writable_journal());
    }

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
     transaction(data);
     data.get_writable_journal().default_checkpoint();
    }
    catch (...)
    {
     cancel_transaction();
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
   const int initial_uncaught_exceptions;

  public:
   Client_Lock(Client &client):
    client(client),
    initial_uncaught_exceptions(std::uncaught_exceptions())
   {
    client.throw_if_pull_when_ahead();
    client.server_checkpoint = client.connection.lock_pull
    (
     client.data.get_writable_journal()
    );
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
     if (std::uncaught_exceptions() > initial_uncaught_exceptions)
      client.cancel_transaction();
     else
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
