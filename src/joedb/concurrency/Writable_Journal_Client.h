#ifndef joedb_Writable_Journal_Client_declared
#define joedb_Writable_Journal_Client_declared

#include "joedb/concurrency/Writable_Client.h"

namespace joedb
{
 namespace detail
 {
  class Writable_Journal_Client_Data
  {
   protected:
    Writable_Journal data_journal;

   public:
    Writable_Journal_Client_Data
    (
     Abstract_File &file,
     Recovery recovery
    ):
     data_journal(Journal_Construction_Lock(file, recovery))
    {
    }
  };
 }

 /// @ingroup concurrency
 class Writable_Journal_Client:
  protected detail::Writable_Journal_Client_Data,
  public Writable_Client
 {
  friend class Writable_Journal_Client_Lock;

  protected:
   void read_journal() override
   {
    data_journal.skip_directly_to(data_journal.get_checkpoint());
   }

  public:
   Writable_Journal_Client
   (
    Abstract_File &file,
    Connection &connection = Connection::dummy,
    Content_Check content_check = Content_Check::fast,
    Recovery recovery = Recovery::none
   ):
    Writable_Journal_Client_Data(file, recovery),
    Writable_Client(data_journal, connection, content_check)
   {
    read_journal();
   }

   template<typename F> auto transaction(F transaction)
   {
    return Writable_Client::transaction([this, &transaction]()
    {
     return transaction(data_journal);
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
    return static_cast<Writable_Journal_Client &>(client).data_journal;
   }
 };
}

#endif
