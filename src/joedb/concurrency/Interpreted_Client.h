#ifndef joedb_Interpreted_Client_declared
#define joedb_Interpreted_Client_declared

#include "joedb/concurrency/Interpreted_Client_Data.h"
#include "joedb/concurrency/Client.h"
#include "joedb/T.h"

#include <memory>

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_Client_Parent
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   Interpreted_Client_Data data;
   std::unique_ptr<Connection> connection;

   template<class Connection_Type, class... Arguments>
   Interpreted_Client_Parent
   (
    Generic_File &file, T<Connection_Type> t, Arguments&&... arguments
   ):
    data(file),
    connection(new Connection_Type(data.get_journal(), arguments...))
   {
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_Client:
 ////////////////////////////////////////////////////////////////////////////
  private Interpreted_Client_Parent,
  public Client
 {
  using Interpreted_Client_Parent::data;
  using Interpreted_Client_Parent::connection;

  public:
   template<class Connection_Type, class... Arguments>
   Interpreted_Client
   (
    Generic_File &file, T<Connection_Type> t, Arguments&&... arguments
   ):
    Interpreted_Client_Parent(file, T<Connection_Type>{}, arguments...),
    Client(data, *connection)
   {
   }

   const Database &get_database() const
   {
    return data.get_database();
   }

   template<typename F> void transaction(F transaction)
   {
    Client::transaction([this, &transaction]()
    {
     transaction(get_database(), data.get_multiplexer());
    });
   }
 };
}

#endif
