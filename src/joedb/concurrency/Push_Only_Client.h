#ifndef joedb_Push_Only_Client_declared
#define joedb_Push_Only_Client_declared

#include "joedb/concurrency/Connection.h"
#include "joedb/Exception.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Push_Only_Client
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   Connection &connection;
   Writable_Journal &journal;
   int64_t server_checkpoint;

   void push_unlock()
   {
    connection.push_unlock(journal, server_checkpoint);
    server_checkpoint = journal.get_checkpoint_position();
   }

  public:
   //////////////////////////////////////////////////////////////////////////
   Push_Only_Client
   //////////////////////////////////////////////////////////////////////////
   (
    Connection &connection,
    Writable_Journal &journal
   ):
    connection(connection),
    journal(journal)
   {
    server_checkpoint = connection.handshake(journal);
    const int64_t client_checkpoint = journal.get_checkpoint_position();

    if
    (
     !connection.check_matching_content
     (
      journal,
      std::min(server_checkpoint, client_checkpoint)
     )
    )
    {
     throw Exception("Client data does not match the server");
    }
   }

   //////////////////////////////////////////////////////////////////////////
   int64_t get_checkpoint_difference() const
   //////////////////////////////////////////////////////////////////////////
   {
    return journal.get_checkpoint_position() - server_checkpoint;
   }

   //////////////////////////////////////////////////////////////////////////
   void push()
   //////////////////////////////////////////////////////////////////////////
   {
    if (get_checkpoint_difference() > 0)
     push_unlock();
   }
 };
}

#endif
