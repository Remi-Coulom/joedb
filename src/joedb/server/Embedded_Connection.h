#ifndef joedb_Embedded_Connection_declared
#define joedb_Embedded_Connection_declared

#include "joedb/server/Connection.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Embedded_Connection: public Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Journal_File server_journal;

   //////////////////////////////////////////////////////////////////////////
   void pull(Journal_File &client_journal) override
   //////////////////////////////////////////////////////////////////////////
   {
    const int64_t client_position = client_journal.get_checkpoint_position();
    const int64_t server_position = server_journal.get_checkpoint_position();

    if (client_position < server_position)
     client_journal.append_raw_tail
     (
      server_journal.get_raw_tail(client_position)
     );
   }

   //////////////////////////////////////////////////////////////////////////
   void lock_pull(Journal_File &client_journal) override
   //////////////////////////////////////////////////////////////////////////
   {
    pull(client_journal);
   }

   //////////////////////////////////////////////////////////////////////////
   void push_unlock(Readonly_Journal &client_journal) override
   //////////////////////////////////////////////////////////////////////////
   {
    const int64_t client_position = client_journal.get_checkpoint_position();
    const int64_t server_position = server_journal.get_checkpoint_position();

    if (server_position < client_position)
     server_journal.append_raw_tail
     (
      client_journal.get_raw_tail(server_position)
     );
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
