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
 class Readonly_Client: protected Readonly_Journal, public Client
 {
  public:
   Readonly_Client
   (
    Buffered_File &file,
    Connection &connection,
    Content_Check content_check = Content_Check::fast,
    Recovery recovery = Recovery::none
   ):
    Readonly_Journal(Journal_Construction_Lock(file, recovery)),
    Client(*this, connection, content_check)
   {
    Client::push(get_connection_checkpoint(), Unlock_Action::keep_locked);
    read_journal();
   }

   int64_t pull
   (
    std::chrono::milliseconds wait = std::chrono::milliseconds(0)
   ) override
   {
    const int64_t result = Readonly_Journal::pull();
    if (result)
     read_journal();
    return result;
   }

   int64_t push_if_ahead(int64_t until)
   {
    pull();
    if (Readonly_Journal::get_checkpoint() > get_connection_checkpoint())
     return Client::push(until, Unlock_Action::keep_locked);
    else
     return get_connection_checkpoint();
   }

   int64_t push_if_ahead() override
   {
    return push_if_ahead(Readonly_Journal::get_checkpoint());
   }

   ~Readonly_Client() override
   {
    try {connection.unlock();} catch (...) {}
   }
 };
}

#endif
