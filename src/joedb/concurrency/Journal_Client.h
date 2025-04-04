#ifndef joedb_Journal_Client_declared
#define joedb_Journal_Client_declared

#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/Writable_Journal_Client_Data.h"

namespace joedb
{
 /// \ingroup concurrency
 class Journal_Client:
  public Writable_Journal_Client_Data,
  public Client
 {
  public:
   Journal_Client
   (
    Buffered_File &file,
    Connection &connection,
    bool content_check = true
   ):
    Writable_Journal_Client_Data(file),
    Client(*this, connection, content_check)
   {
   }

   template<typename F> void transaction(F transaction)
   {
    Client::transaction([&transaction](Client_Data &data)
    {
     data.get_writable_journal().seek_to_checkpoint();
     transaction(data.get_writable_journal());
    });
   }
 };
}

#endif
