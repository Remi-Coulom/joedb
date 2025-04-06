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
   Writable_Journal &get_writable_journal() override {return journal;}

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
   }

   bool is_readonly() const override {return false;}

   template<typename F> void transaction(F transaction)
   {
    Client::transaction([this, &transaction]()
    {
     journal.seek_to_checkpoint();
     transaction(journal);
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
    client.journal.seek_to_checkpoint();
   }

   Writable_Journal &get_journal()
   {
    return static_cast<Writable_Journal_Client &>(client).get_writable_journal();
   }
 };
}

#endif
