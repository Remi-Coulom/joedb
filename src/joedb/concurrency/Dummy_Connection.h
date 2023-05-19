#ifndef joedb_Dummy_Connection_declared
#define joedb_Dummy_Connection_declared

#include "joedb/concurrency/Connection.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Dummy_Connection: public Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   Dummy_Connection(Writable_Journal &client_journal):
    Connection(client_journal)
   {
   }

   int64_t handshake() final
   {
    return client_journal.get_checkpoint_position();
   }

   bool check_matching_content(int64_t server_checkpoint) final
   {
    return true;
   }

   void lock() final
   {
   }

   void unlock() final
   {
   }

   int64_t pull() final
   {
    return client_journal.get_checkpoint_position();
   }

   void push(int64_t server_checkpoint, bool unlock_after) final
   {
   }
 };
}

#endif
