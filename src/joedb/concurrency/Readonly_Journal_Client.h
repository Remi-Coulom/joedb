#ifndef joedb_Readonly_Journal_Client_declared
#define joedb_Readonly_Journal_Client_declared

#include "joedb/concurrency/Readonly_Client.h"

namespace joedb
{
 namespace detail
 {
  class Readonly_Journal_Client_Data
  {
   protected:
    Readonly_Journal data_journal;

   public:
    Readonly_Journal_Client_Data(Buffered_File &file): data_journal(file) {}
  };
 }

 /// @ingroup concurrency
 class Readonly_Journal_Client:
  protected detail::Readonly_Journal_Client_Data,
  public Readonly_Client
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
    Readonly_Client(data_journal, connection, content_check)
   {
   }
 };
}

#endif
