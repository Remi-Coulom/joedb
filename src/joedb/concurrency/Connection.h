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
    Readonly_Journal &client_journal,
    bool content_check
   );

   /// @retval server_checkpoint
   virtual int64_t pull
   (
    Writable_Journal &client_journal,
    std::chrono::milliseconds wait = std::chrono::milliseconds(0)
   );

   /// @retval server_checkpoint
   virtual int64_t lock_pull
   (
    Writable_Journal &client_journal,
    std::chrono::milliseconds wait = std::chrono::milliseconds(0)
   );

   /// @retval server_checkpoint
   virtual int64_t push_until
   (
    Readonly_Journal &client_journal,
    int64_t from_checkpoint,
    int64_t until_checkpoint,
    bool unlock_after
   );

   /// @retval server_checkpoint
   int64_t push
   (
    Readonly_Journal &client_journal,
    int64_t from_checkpoint,
    bool unlock_after
   )
   {
    return push_until
    (
     client_journal,
     from_checkpoint,
     std::numeric_limits<int64_t>::max(),
     unlock_after
    );
   }

   virtual void unlock();

   virtual bool is_pullonly() const;

   virtual ~Connection();
 };
}

#endif
