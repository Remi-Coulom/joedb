#ifndef joedb_Client_declared
#define joedb_Client_declared

#include "joedb/concurrency/Connection.h"

namespace joedb
{
 /// Handle concurrent access to a file with a joedb::Connection
 ///
 /// @ingroup concurrency
 class Client
 {
  protected:
   virtual void read_journal() {}
   Readonly_Journal &journal;
   Connection &connection;
   int64_t connection_checkpoint;

   int64_t push(int64_t until, Unlock_Action unlock_action)
   {
    connection_checkpoint = connection.push
    (
     journal,
     connection_checkpoint,
     until,
     unlock_action
    );

    return connection_checkpoint;
   }

   int64_t push(Unlock_Action unlock_action)
   {
    return push(journal.get_checkpoint(), unlock_action);
   }

  public:
   //////////////////////////////////////////////////////////////////////////
   Client
   //////////////////////////////////////////////////////////////////////////
   (
    Readonly_Journal &journal,
    Connection &connection,
    Content_Check content_check
   ):
    journal(journal),
    connection(connection),
    connection_checkpoint(connection.handshake(journal, content_check))
   {
   }

   const Readonly_Journal &get_journal() const
   {
    return journal;
   }

   bool is_shared() const
   {
    return journal.is_shared();
   }

   bool is_pullonly() const
   {
    return connection.is_pullonly();
   }

   int64_t get_journal_checkpoint() const
   {
    return get_journal().get_checkpoint();
   }

   std::string read_blob(Blob blob) const
   {
    return get_journal().get_file().read_blob(blob);
   }

   int64_t get_connection_checkpoint() const
   {
    return connection_checkpoint;
   }

   int64_t get_checkpoint_difference() const
   {
    return get_journal_checkpoint() - connection_checkpoint;
   }

   virtual int64_t push_if_ahead() = 0;
   virtual int64_t pull
   (
    std::chrono::milliseconds wait = std::chrono::milliseconds(0)
   ) = 0;

   virtual ~Client();
 };
}

#endif
