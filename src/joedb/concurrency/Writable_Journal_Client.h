#ifndef joedb_Writable_Journal_Client_declared
#define joedb_Writable_Journal_Client_declared

#include "joedb/concurrency/Client.h"

namespace joedb
{
 namespace detail
 {
  class Writable_Journal_Client_Data
  {
   protected:
    Writable_Journal journal;

   public:
    Writable_Journal_Client_Data(Buffered_File &file): journal(file) {}
  };
 }

 /// @ingroup concurrency
 class Writable_Journal_Client:
  protected detail::Writable_Journal_Client_Data,
  public Client
 {
  friend class Writable_Journal_Client_Lock;

  protected:
   void read_journal() override
   {
    journal.skip_directly_to(journal.get_checkpoint_position());
   }

  public:
   Writable_Journal_Client
   (
    Buffered_File &file,
    Connection &connection,
    bool content_check = true
   ):
    Writable_Journal_Client_Data(file),
    Client(journal, connection, content_check)
   {
    read_journal();
   }

   template<typename F> auto transaction(F transaction)
   {
    return Client::transaction([this, &transaction]()
    {
     return transaction(journal);
    });
   }
 };

 /// @ingroup concurrency
 class Writable_Journal_Client_Lock: public Client_Lock
 {
  public:
   Writable_Journal_Client_Lock(Writable_Journal_Client &client):
    Client_Lock(client)
   {
   }

   Writable_Journal &get_journal()
   {
    JOEDB_ASSERT(is_locked());
    return static_cast<Writable_Journal_Client &>(client).journal;
   }
 };
}

#endif
