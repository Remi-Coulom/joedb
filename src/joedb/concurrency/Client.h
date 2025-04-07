#ifndef joedb_Client_declared
#define joedb_Client_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/error/Posthumous_Thrower.h"

namespace joedb
{
 /// Handle concurrent access to a file with a joedb::Connection
 /// @ingroup concurrency
 class Client
 {
  friend class Client_Lock;

  protected:
   virtual void read_journal() = 0;

   template<typename F> void transaction(F transaction)
   {
    start_transaction();

    try
    {
     transaction();
     writable_journal->default_checkpoint();
    }
    catch (...)
    {
     cancel_transaction();
     throw;
    }

    end_transaction();
   }

  private:
   Readonly_Journal &readonly_journal;
   Writable_Journal * const writable_journal;

   Connection &connection;
   int64_t server_checkpoint;

   //////////////////////////////////////////////////////////////////////////
   void throw_if_pull_into_uncheckpointed()
   //////////////////////////////////////////////////////////////////////////
   {
    if (readonly_journal.get_position() > get_checkpoint())
     throw Exception("can't pull: client has uncheckpointed data");
   }

   //////////////////////////////////////////////////////////////////////////
   void push(bool unlock_after)
   //////////////////////////////////////////////////////////////////////////
   {
    server_checkpoint = connection.push
    (
     readonly_journal,
     server_checkpoint,
     unlock_after
    );
   }

   //////////////////////////////////////////////////////////////////////////
   void start_transaction()
   //////////////////////////////////////////////////////////////////////////
   {
    throw_if_pull_into_uncheckpointed();
    writable_journal->lock_pull();
    server_checkpoint = connection.lock_pull(*writable_journal);
    read_journal();
   }

   //////////////////////////////////////////////////////////////////////////
   void cancel_transaction()
   //////////////////////////////////////////////////////////////////////////
   {
    connection.unlock();
    writable_journal->flush();
    writable_journal->unlock();
   }

   //////////////////////////////////////////////////////////////////////////
   void end_transaction()
   //////////////////////////////////////////////////////////////////////////
   {
    push_unlock();
    writable_journal->unlock();
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
    readonly_journal(journal),
    writable_journal(journal.get_writable_journal()),
    connection(connection),
    server_checkpoint
    (
     connection.handshake(journal, content_check)
    )
   {
   }

   bool is_readonly() const
   {
    return writable_journal == nullptr;
   }

   const Readonly_Journal &get_journal() const
   {
    return readonly_journal;
   }

   int64_t get_checkpoint() const
   {
    return get_journal().get_checkpoint_position();
   }

   std::string read_blob_data(Blob blob) const
   {
    return get_journal().get_file().read_blob_data(blob);
   }

   int64_t get_server_checkpoint() const
   {
    return server_checkpoint;
   }

   int64_t get_checkpoint_difference() const
   {
    return get_checkpoint() - server_checkpoint;
   }

   /// @param wait indicates how long the connection may wait for new data
   /// @retval pull_size number of bytes pulled
   int64_t pull(std::chrono::milliseconds wait = std::chrono::milliseconds(0))
   {
    const int64_t old_checkpoint = get_checkpoint();

    if (writable_journal)
    {
     throw_if_pull_into_uncheckpointed();
     writable_journal->lock_pull();
     server_checkpoint = connection.pull(*writable_journal, wait);
     writable_journal->unlock(); // even if exception during connection.pull ?
    }
    else
     readonly_journal.pull();

    read_journal();

    return get_checkpoint() - old_checkpoint;
   }

   bool is_pullonly() const {return connection.is_pullonly();}

  public:
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

   virtual ~Client();
 };

 /// Lock object that allows writing to a database managed by a joedb::Client
 ///
 /// This is an alternative to joedb::Client::transaction.
 /// It is more flexible than a transaction, but a bit annoying to use because
 /// push_unlock will take place in the destructor and that makes error
 /// management tricky.
 /// @ingroup concurrency
 class Client_Lock: public Posthumous_Thrower
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
    client.writable_journal->default_checkpoint();
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
      try
      {
       client.writable_journal->default_checkpoint();
      }
      catch (...)
      {
       client.cancel_transaction();
       throw;
      }

      client.end_transaction();
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
