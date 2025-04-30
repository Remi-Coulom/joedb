#ifndef joedb_Readonly_Client_declared
#define joedb_Readonly_Client_declared

#include "joedb/concurrency/Client.h"

namespace joedb
{
 /// Specialized client for read-only files
 ///
 /// @ingroup concurrency
 class Readonly_Client: public Client
 {
  public:
   /// Lock connection right after hanshake
   Readonly_Client
   (
    Readonly_Journal &journal,
    Connection &connection,
    Content_Check content_check = Content_Check::quick
   ):
    Client(journal, connection, content_check)
   {
    push(Unlock_Action::keep_locked);
    if (get_connection_checkpoint() > get_journal_checkpoint())
     throw Exception("Readonly_Client: conflict");
   }

   int64_t pull
   (
    std::chrono::milliseconds wait = std::chrono::milliseconds(0)
   ) override
   {
    const int64_t result = journal.pull();
    read_journal();
    return result;
   }

   using Client::push;
 };
}

#endif
