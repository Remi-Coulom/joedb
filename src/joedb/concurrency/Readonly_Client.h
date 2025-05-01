#ifndef joedb_Readonly_Client_declared
#define joedb_Readonly_Client_declared

#include "joedb/concurrency/Client.h"

namespace joedb
{
 /// Specialized client for read-only files
 ///
 /// This Client has no support for transactions: the connection is locked
 /// in the constructor and unlocked in the destructor. Only allowed
 /// operations are pulling from the journal, and pushing to the connection.
 ///
 /// @ingroup concurrency
 class Readonly_Client: public Client
 {
  public:
   Readonly_Client
   (
    Readonly_Journal &journal,
    Connection &connection,
    Content_Check content_check = Content_Check::quick
   ):
    Client(journal, connection, content_check)
   {
    push();
    read_journal();
   }

   int64_t pull
   (
    std::chrono::milliseconds wait = std::chrono::milliseconds(0)
   ) override
   {
    const int64_t result = journal.pull();
    if (result)
     read_journal();
    return result;
   }

   int64_t push() override
   {
    if (get_journal_checkpoint() > get_connection_checkpoint())
     return Client::push(Unlock_Action::keep_locked);
    else
     return get_connection_checkpoint();
   }

   ~Readonly_Client() override
   {
    try {connection.unlock();} catch (...) {}
   }
 };
}

#endif
