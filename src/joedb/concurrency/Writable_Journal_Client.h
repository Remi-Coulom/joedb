#ifndef joedb_Writable_Journal_Client_declared
#define joedb_Writable_Journal_Client_declared

#include "joedb/concurrency/Writable_Client.h"

namespace joedb
{
 /// @ingroup concurrency
 class Writable_Journal_Client: public Writable_Client
 {
  friend class Writable_Journal_Client_Lock;

  protected:
   void read_journal() override
   {
    skip_directly_to(get_checkpoint());
   }

  public:
   Writable_Journal_Client
   (
    Buffered_File &file,
    Connection &connection,
    Content_Check content_check = Content_Check::quick
   ):
    Writable_Client(file, connection, content_check)
   {
    read_journal();
   }

   template<typename F> auto transaction(F transaction)
   {
    return Writable_Client::transaction([this, &transaction]()
    {
     return transaction(get_writable_journal());
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
    JOEDB_DEBUG_ASSERT(locked);
    return static_cast<Writable_Journal_Client &>(client).get_writable_journal();
   }
 };
}

#endif
