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
  protected:
   void content_mismatch()
   {
    throw Exception("Client data does not match the server");
   }

  public:
   Writable_Journal &client_journal;

  public:
   Connection(Writable_Journal &client_journal):
    client_journal(client_journal)
   {}

   virtual int64_t handshake() = 0;

   void lock() override = 0;
   void unlock() override = 0;

   virtual int64_t pull() = 0;

   virtual int64_t lock_pull()
   {
    lock();
    return pull();
   }

   virtual void push(int64_t server_checkpoint, bool unlock_after) = 0;
 };
}

#endif
