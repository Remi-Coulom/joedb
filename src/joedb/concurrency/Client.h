#ifndef joedb_Client_declared
#define joedb_Client_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/error/Destructor_Logger.h"

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
    Journal_Lock lock(*writable_journal);

    start_transaction();

    try
    {
     transaction();
     writable_journal->soft_checkpoint();
    }
    catch (...)
    {
     connection.unlock();
     throw;
    }

    push_unlock();
   }

  private:
   Readonly_Journal &readonly_journal;
   Writable_Journal * const writable_journal;

   Connection &connection;
   int64_t server_checkpoint;

   //////////////////////////////////////////////////////////////////////////
   void push(bool unlock_after)
   //////////////////////////////////////////////////////////////////////////
   {
    server_checkpoint = connection.push
    (
     readonly_journal,
     server_checkpoint,
     readonly_journal.get_checkpoint_position(),
     unlock_after
    );
   }

   //////////////////////////////////////////////////////////////////////////
   void push_and_keep_locked()
   //////////////////////////////////////////////////////////////////////////
   {
    push(false);
   }

   //////////////////////////////////////////////////////////////////////////
   void start_transaction()
   //////////////////////////////////////////////////////////////////////////
   {
    server_checkpoint = connection.pull(true, true, *writable_journal);
    read_journal();
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
     Journal_Lock lock(*writable_journal);
     server_checkpoint = connection.pull(false, true, *writable_journal, wait);
    }
    else
     readonly_journal.pull();

    read_journal();

    return get_checkpoint() - old_checkpoint;
   }

   void push_unlock()
   {
    push(true);
   }

   virtual ~Client();
 };

 /// Lock object that allows writing to a database managed by a joedb::Client
 ///
 /// At the end of the life of this object, right before destruction, you
 /// should call either @ref unlock to cancel the transaction, or
 /// @ref push_unlock to confirm it. If you fail to do so, the destructor
 /// will call @ref unlock. But calling unlock explicitly is better if possible,
 /// because it can throw exceptions, unlike the destructor.
 ///
 /// @ingroup concurrency
 class Client_Lock
 {
  private:
   bool locked;

  protected:
   Client &client;
   Journal_Lock journal_lock;
   bool is_locked() const {return locked;}

  public:
   Client_Lock(Client &client):
    locked(true),
    client(client),
    journal_lock(*client.writable_journal)
   {
    client.start_transaction();
   }

   Client_Lock(const Client_Lock &) = delete;
   Client_Lock &operator=(const Client_Lock &) = delete;

   /// Checkpoint current journal, and push to the connection
   ///
   /// Unlike @ref push_unlock, you can call this function multiple
   /// times during the life of the lock.
   void push()
   {
    JOEDB_ASSERT(is_locked());
    client.writable_journal->soft_checkpoint();
    client.push_and_keep_locked();
   }

   /// Confirm the transaction right before lock destruction
   ///
   /// Destruction should happen right after this function.
   /// Do not call any other member function after this one.
   void push_unlock()
   {
    JOEDB_ASSERT(is_locked());
    client.writable_journal->soft_checkpoint();
    client.push_unlock();
    locked = false;
   }

   /// Cancel the transaction right before lock destruction
   ///
   /// Destruction should happen right after this function.
   /// Do not call any other member function after this one.
   void unlock()
   {
    JOEDB_ASSERT(is_locked());
    client.connection.unlock();
    locked = false;
   }

   ~Client_Lock()
   {
    if (locked)
    {
     Destructor_Logger::write("locked in destructor, cancelling transaction");
     try { unlock(); } catch (...) {}
    }
   }
 };
}

#endif
