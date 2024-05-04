#ifndef joedb_Client_declared
#define joedb_Client_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/concurrency/Client_Data.h"
#include "joedb/Posthumous_Thrower.h"

namespace joedb
{
 class Client;

 ////////////////////////////////////////////////////////////////////////////
 class Pullonly_Client
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   Client_Data &data;
   Pullonly_Connection &connection;
   int64_t server_checkpoint;

   //////////////////////////////////////////////////////////////////////////
   void throw_if_pull_when_ahead()
   //////////////////////////////////////////////////////////////////////////
   {
    if
    (
     get_checkpoint() > server_checkpoint ||
     data.get_readonly_journal().get_position() > server_checkpoint
    )
    {
     throw Exception("can't pull: client is ahead of server");
    }
   }

  public:
   //////////////////////////////////////////////////////////////////////////
   Pullonly_Client
   //////////////////////////////////////////////////////////////////////////
   (
    Client_Data &data,
    Pullonly_Connection &connection
   ):
    data(data),
    connection(connection),
    server_checkpoint(connection.handshake(data.get_readonly_journal()))
   {
   }

   Client_Data &get_data() const {return data;}
   const Readonly_Journal &get_journal() {return data.get_readonly_journal();}
   bool is_readonly() const {return data.is_readonly();}
   virtual Client *get_push_client() {return nullptr;}

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

   virtual ~Pullonly_Client() = default;
 };

 ////////////////////////////////////////////////////////////////////////////
 class Client: public Pullonly_Client
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Client_Lock;

  private:
   Connection &connection;

   //////////////////////////////////////////////////////////////////////////
   void push(bool unlock_after)
   //////////////////////////////////////////////////////////////////////////
   {
    server_checkpoint = connection.push
    (
     data.get_readonly_journal(),
     server_checkpoint,
     unlock_after
    );
   }

   //////////////////////////////////////////////////////////////////////////
   void start_transaction()
   //////////////////////////////////////////////////////////////////////////
   {
    throw_if_pull_when_ahead();
    server_checkpoint = connection.lock_pull(data.get_writable_journal());
   }

   //////////////////////////////////////////////////////////////////////////
   void cancel_transaction()
   //////////////////////////////////////////////////////////////////////////
   {
    connection.unlock(data.get_writable_journal());
    data.get_writable_journal().flush();
   }

  public:
   //////////////////////////////////////////////////////////////////////////
   Client
   //////////////////////////////////////////////////////////////////////////
   (
    Client_Data &data,
    Connection &connection
   ):
    Pullonly_Client(data, connection),
    connection(connection)
   {
   }

   Client *get_push_client() override {return this;}

   //////////////////////////////////////////////////////////////////////////
   void push_unlock()
   //////////////////////////////////////////////////////////////////////////
   {
    push(true);
   }

   //////////////////////////////////////////////////////////////////////////
   void push_and_keep_locked()
   //////////////////////////////////////////////////////////////////////////
   {
    push(false);
   }

   //////////////////////////////////////////////////////////////////////////
   template<typename F> void transaction(F transaction)
   //////////////////////////////////////////////////////////////////////////
   {
    start_transaction();

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

   Client_Lock(const Client_Lock &) = delete;

  public:
   Client_Lock(Client &client):
    client(client),
    initial_uncaught_exceptions(std::uncaught_exceptions())
   {
    client.start_transaction();
   }

   Writable_Journal &get_journal()
   {
    return client.data.get_writable_journal();
   }

   void push()
   {
    client.push_and_keep_locked();
   }

   ~Client_Lock()
   {
    try
    {
     if (std::uncaught_exceptions() > initial_uncaught_exceptions)
      client.cancel_transaction();
     else
     {
      get_journal().default_checkpoint();
      client.push_unlock();
     }
    }
    catch (...)
    {
     postpone_exception();
    }
   }
 };
}

#endif
