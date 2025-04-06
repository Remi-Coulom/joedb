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

  protected:
   Readonly_Journal &get_readonly_journal() override {return journal;}

  public:
   Readonly_Journal_Client
   (
    Buffered_File &file,
    Connection &connection,
    bool content_check = true
   ):
    Readonly_Journal_Client_Data(file),
    Client(journal, connection, content_check)
   {
   }

   bool is_readonly() const override {return true;}
 };
}

#endif
