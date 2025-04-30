#ifndef joedb_Connection_declared
#define joedb_Connection_declared

#include "joedb/journal/Writable_Journal.h"

#include <chrono>

namespace joedb
{
 /// @ingroup concurrency
 class Content_Mismatch: public Exception
 {
  public:
   Content_Mismatch();
 };

 /// @ingroup concurrency
 enum class Content_Check
 {
  none,
  quick,
  full
 };

 /// @ingroup concurrency
 enum class Data_Transfer
 {
  without_data = 0,
  with_data = 1
 };

 /// @ingroup concurrency
 enum class Lock_Action
 {
  no_locking = 0,
  lock_before = 1
 };

 /// @ingroup concurrency
 enum class Unlock_Action
 {
  keep_locked = 0,
  unlock_after = 1
 };

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
   /// @param content_check indicates how matching content is tested
   ///
   /// @retval connection checkpoint
   virtual int64_t handshake
   (
    Readonly_Journal &client_journal,
    Content_Check content_check
   );

   /// Pull from the connection
   ///
   /// @param lock_action whether the connection should be locked before pulling
   /// @param data_transfer whether data should be transferred
   /// @param client_journal journal to pull into
   /// @param wait duration during which the connection may wait
   /// for new data if the pull would otherwise be empty
   ///
   /// @retval server checkpoint
   virtual int64_t pull
   (
    Lock_Action lock_action,
    Data_Transfer data_transfer,
    Writable_Journal &client_journal,
    std::chrono::milliseconds wait = std::chrono::milliseconds(0)
   );

   /// Push new data to the connection
   ///
   /// @retval server checkpoint
   virtual int64_t push
   (
    Readonly_Journal &client_journal,
    int64_t from,
    int64_t until,
    Unlock_Action unlock_action
   );

   /// Unlock the connection
   virtual void unlock();

   virtual ~Connection();
 };
}

#endif
