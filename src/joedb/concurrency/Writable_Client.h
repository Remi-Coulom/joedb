#ifndef joedb_Writable_Client_declared
#define joedb_Writable_Client_declared

#include "joedb/concurrency/Client.h"
#include "joedb/error/Destructor_Logger.h"

namespace joedb
{
 /// Writable specialization of Client
 ///
 /// @ingroup concurrency
 class Writable_Client: public Client
 {
  friend class Client_Lock;

  protected:
   template<typename F> auto transaction(F transaction)
   {
    const Journal_Lock lock(get_writable_journal());

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
     const T result = [&]()
     {
      try
      {
       const T inner_result = transaction();
       do_checkpoint();
       return inner_result;
      }
      catch (...)
      {
       connection.unlock();
       throw;
      }
     } ();

     push_unlock();
     return result;
    }
   }

  private:
   Writable_Journal &get_writable_journal()
   {
    return static_cast<Writable_Journal&>(Client::journal);
   }

   bool use_valid_data = false;
   bool use_timestamp = false;
   bool use_hard_checkpoint = false;

   void do_checkpoint()
   {
    if (use_valid_data)
     get_writable_journal().valid_data();

    if (use_timestamp)
     get_writable_journal().timestamp(std::time(nullptr));

    get_writable_journal().soft_checkpoint();

    if (use_hard_checkpoint)
     get_writable_journal().hard_checkpoint();
   }

   void start_transaction()
   {
    connection_checkpoint = connection.pull
    (
     Lock_Action::lock_before,
     Data_Transfer::with_data,
     get_writable_journal()
    );

    read_journal();
   }

  public:
   Writable_Client
   (
    Writable_Journal &journal,
    Connection &connection = Connection::dummy,
    Content_Check content_check = Content_Check::fast
   ): Client(journal, connection, content_check)
   {
   }

   /// Automatically write valid_data at every checkpoint (default = false)
   void set_valid_data(bool b) {use_valid_data = b;}

   /// Automatically write time stamp at every checkpoint (default = false)
   void set_timestamp(bool b) {use_timestamp = b;}

   /// Use hard checkpoints (default = false)
   void set_hard_checkpoint(bool b) {use_hard_checkpoint = b;}

   /// @param wait indicates how long the connection may wait for new data
   ///
   /// @return number of bytes pulled
   int64_t pull
   (
    std::chrono::milliseconds wait = std::chrono::milliseconds(0)
   ) override
   {
    const int64_t old_checkpoint = get_journal_checkpoint();

    const Journal_Lock lock(get_writable_journal());

    connection_checkpoint = connection.pull
    (
     Lock_Action::no_locking,
     Data_Transfer::with_data,
     get_writable_journal(),
     wait
    );

    read_journal();

    return get_journal_checkpoint() - old_checkpoint;
   }

   void pull_push()
   {
    transaction([](){});
   }

   int64_t push_unlock()
   {
    return Client::push(Unlock_Action::unlock_after);
   }

   int64_t push_if_ahead() override
   {
    if (get_journal_checkpoint() > get_connection_checkpoint())
     return push_unlock();
    else
     return get_connection_checkpoint();
   }
 };

 /// Lock object that allows writing to a database managed by a joedb::Client
 ///
 /// At the end of the life of this object, right before destruction, you
 /// should call either @ref unlock to cancel the transaction, or
 /// @ref push_unlock to confirm it. If you fail to do so, the destructor
 /// will call @ref unlock. But calling unlock explicitly is better,
 /// if possible, because it can throw exceptions, unlike the destructor.
 ///
 /// @ingroup concurrency
 class Client_Lock
 {
  protected:
   Writable_Client &client;
   const Journal_Lock journal_lock;
   bool locked;

  public:
   Client_Lock(Writable_Client &client):
    client(client),
    journal_lock(client.get_writable_journal()),
    locked(true)
   {
    client.start_transaction();
   }

   Client_Lock(const Client_Lock &) = delete;
   Client_Lock &operator=(const Client_Lock &) = delete;

   /// Checkpoint current journal, but do not push yet
   void do_checkpoint()
   {
    JOEDB_DEBUG_ASSERT(locked);
    client.do_checkpoint();
   }

   /// Push if the journal checkpoint is ahead of the connection checkpoint
   ///
   /// This function keeps the connection locked
   void push_if_ahead()
   {
    JOEDB_DEBUG_ASSERT(locked);
    if (client.get_journal_checkpoint() > client.get_connection_checkpoint())
     client.Client::push(Unlock_Action::keep_locked);
   }

   /// Checkpoint current journal, and push to the connection
   ///
   /// Unlike @ref push_unlock, you can call this function multiple
   /// times during the life of the lock.
   void checkpoint_and_push()
   {
    JOEDB_DEBUG_ASSERT(locked);
    client.do_checkpoint();
    client.Client::push(Unlock_Action::keep_locked);
   }

   /// Confirm the transaction right before lock destruction
   ///
   /// Destruction should happen right after this function.
   /// Do not call any other member function after this one.
   void checkpoint_and_push_unlock()
   {
    JOEDB_DEBUG_ASSERT(locked);
    client.do_checkpoint();
    client.Client::push(Unlock_Action::unlock_after);
    locked = false;
   }

   /// Cancel the transaction right before lock destruction
   ///
   /// Destruction should happen right after this function.
   /// Do not call any other member function after this one.
   void unlock()
   {
    JOEDB_DEBUG_ASSERT(locked);
    client.connection.unlock();
    locked = false;
   }

   /// The destructor unlocks the connection if necessary
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
