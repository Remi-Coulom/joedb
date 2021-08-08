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
  friend class Client;

  private:
   virtual int64_t pull(Writable_Journal &client_journal) = 0;

   virtual int64_t lock_pull(Writable_Journal &client_journal) = 0;

   virtual void push_unlock
   (
    Readonly_Journal &client_journal,
    int64_t server_position
   ) = 0;

   void unlock() override = 0;

  public:
   virtual ~Connection() {}
 };
}

#endif
