#ifndef joedb_Client_declared
#define joedb_Client_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/error/Posthumous_Thrower.h"

namespace joedb
{
 /// \addtogroup concurrency
 /// @{
 class Client;

 ////////////////////////////////////////////////////////////////////////////
 class Pullonly_Client
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   virtual Writable_Journal &get_writable_journal()
   {
    throw Assertion_Failure("Client_Data has no writable journal");
   }

   virtual Readonly_Journal &get_readonly_journal()
   {
    return get_writable_journal();
   }

   Pullonly_Connection &connection;
   int64_t server_checkpoint;

   //////////////////////////////////////////////////////////////////////////
   void throw_if_pull_when_ahead()
   //////////////////////////////////////////////////////////////////////////
   {
    if
    (
     get_checkpoint() > server_checkpoint ||
     get_readonly_journal().get_position() > server_checkpoint
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
    Readonly_Journal &journal,
    Pullonly_Connection &connection,
    bool content_check = true
   ):
    connection(connection),
    server_checkpoint
    (
     connection.handshake(journal, content_check)
    )
   {
   }

   virtual Client *get_push_client() {return nullptr;}

   //////////////////////////////////////////////////////////////////////////
   const Readonly_Journal &get_journal() const
   //////////////////////////////////////////////////////////////////////////
   {
    return const_cast<Pullonly_Client *>(this)->get_readonly_journal();
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t get_checkpoint() const
   //////////////////////////////////////////////////////////////////////////
   {
    return get_journal().get_checkpoint_position();
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
   int64_t pull(std::chrono::milliseconds wait = std::chrono::milliseconds(0))
   //////////////////////////////////////////////////////////////////////////
   {
    const int64_t old_checkpoint = get_checkpoint();

    if (is_readonly())
     get_readonly_journal().pull();
    else
    {
     throw_if_pull_when_ahead();
     server_checkpoint = connection.pull(get_writable_journal(), wait);
    }

    return get_checkpoint() - old_checkpoint;
   }

   virtual bool is_readonly() const = 0;

   virtual ~Pullonly_Client();
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
     get_readonly_journal(),
     server_checkpoint,
     unlock_after
    );
   }

   //////////////////////////////////////////////////////////////////////////
   void start_transaction()
   //////////////////////////////////////////////////////////////////////////
   {
    throw_if_pull_when_ahead();
    server_checkpoint = connection.lock_pull(get_writable_journal());
   }

   //////////////////////////////////////////////////////////////////////////
   void cancel_transaction()
   //////////////////////////////////////////////////////////////////////////
   {
    connection.unlock(get_writable_journal());
    get_writable_journal().flush();
   }

  protected:
   //////////////////////////////////////////////////////////////////////////
   template<typename F> void transaction(F transaction)
   //////////////////////////////////////////////////////////////////////////
   {
    start_transaction();

    try
    {
     transaction();
     get_writable_journal().default_checkpoint();
    }
    catch (...)
    {
     cancel_transaction();
     throw;
    }

    push_unlock();
   }

  public:
   //////////////////////////////////////////////////////////////////////////
   Client
   //////////////////////////////////////////////////////////////////////////
   (
    Readonly_Journal &journal,
    Connection &connection,
    bool content_check = true
   ):
    Pullonly_Client(journal, connection, content_check),
    connection(connection)
   {
   }

   //////////////////////////////////////////////////////////////////////////
   Client *get_push_client() override
   //////////////////////////////////////////////////////////////////////////
   {
    if (connection.get_push_connection())
     return this;
    else
     return nullptr;
   }

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
 };

 ////////////////////////////////////////////////////////////////////////////
 class Client_Lock: public Posthumous_Thrower
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   const int initial_uncaught_exceptions;

  protected:
   Client &client;

  public:
   Client_Lock(Client &client):
    initial_uncaught_exceptions(std::uncaught_exceptions()),
    client(client)
   {
    client.start_transaction();
   }

   Client_Lock(const Client_Lock &) = delete;
   Client_Lock &operator=(const Client_Lock &) = delete;

   void push()
   {
    client.get_writable_journal().default_checkpoint();
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
      client.get_writable_journal().default_checkpoint();
      client.push_unlock();
     }
    }
    catch (...)
    {
     postpone_exception();
    }
   }
 };
 /// @}
}

#endif
