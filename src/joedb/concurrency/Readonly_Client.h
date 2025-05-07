#ifndef joedb_Readonly_Client_declared
#define joedb_Readonly_Client_declared

#include "joedb/concurrency/Client.h"

namespace joedb
{
 namespace detail
 {
  class Readonly_Client_Data
  {
   protected:
    Readonly_Journal data_journal;

   public:
    Readonly_Client_Data(Buffered_File &file): data_journal(file) {}
  };
 }

 /// Specialized client for read-only files
 ///
 /// This Client has no support for transactions: the connection is locked
 /// in the constructor and unlocked in the destructor. Only allowed
 /// operations are pulling from the journal, and pushing to the connection.
 ///
 /// @ingroup concurrency
 class Readonly_Client: protected detail::Readonly_Client_Data, public Client
 {
  public:
   Readonly_Client
   (
    Buffered_File &file,
    Connection &connection,
    Content_Check content_check = Content_Check::quick
   ):
    detail::Readonly_Client_Data(file),
    Client(data_journal, connection, content_check)
   {
    Client::push(Unlock_Action::keep_locked);
    read_journal();
   }

   int64_t pull
   (
    std::chrono::milliseconds wait = std::chrono::milliseconds(0)
   ) override
   {
    const int64_t result = data_journal.pull();
    if (result)
     read_journal();
    return result;
   }

   int64_t push_if_ahead() override
   {
    pull();
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
