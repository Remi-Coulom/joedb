#ifndef joedb_Journal_Client_declared
#define joedb_Journal_Client_declared

#include "joedb/concurrency/Journal_Client_Data.h"
#include "joedb/concurrency/Client.h"
#include "joedb/T.h"

#include <memory>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Journal_Client_Parent
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   Journal_Client_Data data;
   std::unique_ptr<Connection> connection;

   template<class Connection_Type, class... Arguments>
   Journal_Client_Parent
   (
    Generic_File &file, T<Connection_Type> t, Arguments&&... arguments
   ):
    data(file),
    connection(new Connection_Type(data.get_journal(), arguments...))
   {
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Journal_Client:
 ////////////////////////////////////////////////////////////////////////////
  private Journal_Client_Parent,
  public Client
 {
  using Journal_Client_Parent::data;
  using Journal_Client_Parent::connection;

  public:
   template<class Connection_Type, class... Arguments>
   Journal_Client
   (
    Generic_File &file, T<Connection_Type> t, Arguments&&... arguments
   ):
    Journal_Client_Parent(file, T<Connection_Type>{}, arguments...),
    Client(data, *connection)
   {
   }

   template<typename F> void transaction(F transaction)
   {
    Client::transaction([this, &transaction]()
    {
     transaction(data.get_journal());
    });
   }
 };
}

#endif
