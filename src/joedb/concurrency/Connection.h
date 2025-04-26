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
   ///
   /// @param client_journal may be used to check matching content
   /// @param content_check indicates whether matching content is tested
   ///
   /// @retval connection checkpoint (-1 if not available)
   virtual int64_t handshake
   (
    Readonly_Journal &client_journal,
    bool content_check
   );

   /// Pull from the connection
   ///
   /// @param lock_before whether the connection should be locked
   /// @param write_data whether data should be written into @ref client_journal
   /// @param client_journal journal to pull into
   /// @param wait duration during which the connection may wait
   /// for new data if the pull would otherwise be empty
   ///
   /// @retval server checkpoint (-1 if not available)
   virtual int64_t pull
   (
    bool lock_before,
    bool write_data,
    Writable_Journal &client_journal,
    std::chrono::milliseconds wait = std::chrono::milliseconds(0)
   );

   /// Push new data to the connection
   ///
   /// @retval server checkpoint (-1 if not available)
   virtual int64_t push
   (
    Readonly_Journal &client_journal,
    int64_t from,
    int64_t until,
    bool unlock_after
   );

   /// Unlock the connection
   virtual void unlock();

   virtual ~Connection();
 };
}

#endif
