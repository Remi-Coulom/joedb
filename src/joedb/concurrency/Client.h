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

   template<typename F> auto transaction(F transaction)
   {
    Journal_Lock lock(*writable_journal);

    start_transaction();

    using T = decltype(transaction());

    if constexpr (std::is_void<T>::value)
    {
     try
     {
      transaction();
      do_checkpoint();
     }
     catch (...)
     {
      connection.unlock();
      throw;
     }

     push_unlock();
    }
    else
    {
     T result;

     try
     {
      result = transaction();
      do_checkpoint();
     }
     catch (...)
     {
      connection.unlock();
      throw;
     }

     push_unlock();
     return result;
    }
   }

  private:
   Readonly_Journal &readonly_journal;
   Writable_Journal * const writable_journal;

   Connection &connection;
   int64_t server_checkpoint;

   bool use_valid_data = false;
   bool use_timestamp = false;
   bool use_hard_checkpoint = false;

   //////////////////////////////////////////////////////////////////////////
   void do_checkpoint()
   //////////////////////////////////////////////////////////////////////////
   {
    if (use_valid_data)
     writable_journal->valid_data();

    if (use_timestamp)
     writable_journal->timestamp(std::time(nullptr));

    writable_journal->soft_checkpoint();

    if (use_hard_checkpoint)
     writable_journal->hard_checkpoint();
   }

   //////////////////////////////////////////////////////////////////////////
   void push(bool unlock_after)
   //////////////////////////////////////////////////////////////////////////
   {
    server_checkpoint = connection.push
    (
     readonly_journal,
     server_checkpoint,
     readonly_journal.get_checkpoint(),
     unlock_after
    );
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

   /// Automatically write valid_data at every checkpoint (default = false)
   void set_valid_data(bool b) {use_valid_data = b;}

   /// Automatically write time stamp at every checkpoint (default = false)
   void set_timestamp(bool b) {use_timestamp = b;}

   /// Use hard checkpoints (default = false)
   void set_hard_checkpoint(bool b) {use_hard_checkpoint = b;}

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
    return get_journal().get_checkpoint();
   }

   std::string read_blob(Blob blob) const
   {
    return get_journal().get_file().read_blob(blob);
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
    client.do_checkpoint();
    client.push(false);
   }

   /// Confirm the transaction right before lock destruction
   ///
   /// Destruction should happen right after this function.
   /// Do not call any other member function after this one.
   void push_unlock()
   {
    JOEDB_ASSERT(is_locked());
    client.do_checkpoint();
    client.push(true);
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
