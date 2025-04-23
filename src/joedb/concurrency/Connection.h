#ifndef joedb_Connection_declared
#define joedb_Connection_declared

#include "joedb/journal/Writable_Journal.h"

#include <chrono>

namespace joedb
{
 /// @ingroup concurrency
 class Connection
 {
  protected:
   static void content_mismatch();

  public:
   /// Called during Client construction
   /// @param client_journal may be used to check matching content
   /// @param content_check indicates whether matching content is tested
   /// @retval server_checkpoint
   virtual int64_t handshake
   (
    const Readonly_Journal &client_journal,
    bool content_check
   );

   /// Pull new data from the connection
   /// @param client_journal: journal to pull into
   /// @param wait: duration during which the connection may wait
   /// for new data if the pull would otherwise be empty
   /// @retval server_checkpoint
   virtual int64_t pull
   (
    Writable_Journal &client_journal,
    std::chrono::milliseconds wait = std::chrono::milliseconds(0)
   );

   /// Fused lock_pull, executed at the start of a write transaction
   /// @retval server_checkpoint
   virtual int64_t lock_pull
   (
    Writable_Journal &client_journal,
    std::chrono::milliseconds wait = std::chrono::milliseconds(0)
   );

   /// Get new connection checkpoint without pulling
   /// @retval server_checkpoint
   virtual int64_t get_checkpoint
   (
    const Readonly_Journal &client_journal,
    std::chrono::milliseconds wait = std::chrono::milliseconds(0)
   );

   /// Push new data to the connection
   /// @retval server_checkpoint
   virtual int64_t push_until
   (
    const Readonly_Journal &client_journal,
    int64_t from_checkpoint,
    int64_t until_checkpoint,
    bool unlock_after
   );

   /// Shortcut to call @ref push_until until the client checkpoint
   /// @retval server_checkpoint
   int64_t push
   (
    const Readonly_Journal &client_journal,
    int64_t from_checkpoint,
    bool unlock_after
   )
   {
    return push_until
    (
     client_journal,
     from_checkpoint,
     client_journal.get_checkpoint_position(),
     unlock_after
    );
   }

   /// Can be used to cancel a transaction without pushing.
   virtual void unlock();

   virtual ~Connection();
 };
}

#endif
