#ifndef joedb_Embedded_Connection_declared
#define joedb_Embedded_Connection_declared

#include "joedb/concurrency/Connection.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Embedded_Connection: public Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Writable_Journal server_journal;

   void lock() override {}
   void unlock() override {}

   //////////////////////////////////////////////////////////////////////////
   int64_t pull(Writable_Journal &client_journal) override
   //////////////////////////////////////////////////////////////////////////
   {
    const int64_t client_position = client_journal.get_checkpoint_position();
    const int64_t server_position = server_journal.get_checkpoint_position();

    if (client_position < server_position)
     client_journal.append_raw_tail
     (
      server_journal.get_raw_tail(client_position)
     );

    return server_position;
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t lock_pull(Writable_Journal &client_journal) override
   //////////////////////////////////////////////////////////////////////////
   {
    return pull(client_journal);
   }

   //////////////////////////////////////////////////////////////////////////
   void push_unlock
   //////////////////////////////////////////////////////////////////////////
   (
    Readonly_Journal &client_journal,
    const int64_t server_position
   ) override
   {
    const int64_t client_position = client_journal.get_checkpoint_position();

    if (server_position < client_position)
     server_journal.append_raw_tail
     (
      client_journal.get_raw_tail(server_position)
     );
   }

   //////////////////////////////////////////////////////////////////////////
   bool check_hash(Readonly_Journal &client_journal) override
   //////////////////////////////////////////////////////////////////////////
   {
    const int64_t checkpoint = client_journal.get_checkpoint_position();
    return
     client_journal.get_hash(checkpoint) ==
     server_journal.get_hash(checkpoint);
   }

  public:
   //////////////////////////////////////////////////////////////////////////
   Embedded_Connection(Generic_File &file):
   //////////////////////////////////////////////////////////////////////////
    server_journal(file)
   {
   }
 };
}

#endif
