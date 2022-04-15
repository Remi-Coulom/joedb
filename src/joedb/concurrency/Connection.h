#ifndef joedb_Connection_declared
#define joedb_Connection_declared

#include "joedb/journal/Writable_Journal.h"
#include "joedb/concurrency/Mutex.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Connection: public Mutex
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   virtual int64_t handshake(Readonly_Journal &client_journal) = 0;

   virtual bool check_matching_content
   (
    Readonly_Journal &client_journal,
    int64_t checkpoint
   ) = 0;

   void lock() override = 0;
   void unlock() override = 0;

   virtual int64_t pull(Writable_Journal &client_journal) = 0;

   virtual int64_t lock_pull(Writable_Journal &client_journal)
   {
    lock();
    return pull(client_journal);
   }

   virtual void push_unlock
   (
    Readonly_Journal &client_journal,
    int64_t server_checkpoint
   ) = 0;

   virtual ~Connection() = default;
 };
}

#endif
