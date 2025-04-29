#ifndef joedb_Readonly_Journal_Client_declared
#define joedb_Readonly_Journal_Client_declared

#include "joedb/concurrency/Client.h"

namespace joedb
{
 namespace detail
 {
  class Readonly_Journal_Client_Data
  {
   protected:
    Readonly_Journal journal;

   public:
    Readonly_Journal_Client_Data(Buffered_File &file): journal(file) {}
  };
 }

 /// @ingroup concurrency
 class Readonly_Journal_Client:
  protected detail::Readonly_Journal_Client_Data,
  public Client
 {
  friend class Readonly_Journal_Client_Lock;

  public:
   Readonly_Journal_Client
   (
    Buffered_File &file,
    Connection &connection,
    Content_Check content_check = Content_Check::quick
   ):
    Readonly_Journal_Client_Data(file),
    Client(journal, connection, content_check)
   {
   }
 };
}

#endif
