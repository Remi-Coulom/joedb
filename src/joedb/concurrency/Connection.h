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
   /// Called by @ref handshake when the file and the connection do not match
   static void content_mismatch();

  public:
   /// Called during Client construction
   /// @param client_journal may be used to check matching content
   /// @param content_check indicates whether matching content is tested
   /// @retval connection checkpoint (-1 if not available)
   virtual int64_t handshake
   (
    const Readonly_Journal &client_journal,
    bool content_check
   );

   /// Pull new data from the connection
   /// @param lock_before whether the connection should be locked
   /// @param wait duration during which the connection may wait
   /// for new data if the pull would otherwise be empty
   /// @param client_journal journal to pull into
   /// @retval server checkpoint (-1 if not available)
   virtual int64_t pull
   (
    bool lock_before,
    std::chrono::milliseconds wait,
    Writable_Journal *client_journal
   );

   /// Push new data to the connection
   /// @retval server checkpoint (-1 if not available)
   virtual int64_t push
   (
    const Readonly_Journal *client_journal,
    int64_t from,
    int64_t until,
    bool unlock_after
   );

   /// unlock
   void unlock()
   {
    push(nullptr, -1, -1, true);
   }

   int64_t lock_pull(Writable_Journal &journal)
   {
    return pull(true, std::chrono::milliseconds(0), &journal);
   }

   virtual ~Connection();
 };
}

#endif
