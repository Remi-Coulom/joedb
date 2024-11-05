#ifndef joedb_Interpreted_Client_declared
#define joedb_Interpreted_Client_declared

#include "joedb/concurrency/Client.h"
#include "joedb/concurrency/Interpreted_Client_Data.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Interpreted_Client:
 ////////////////////////////////////////////////////////////////////////////
  public Writable_Interpreted_Client_Data,
  public Client
 {
  public:
   Interpreted_Client
   (
    Generic_File &file,
    Connection &connection,
    bool content_check = true
   ):
    Writable_Interpreted_Client_Data(file),
    Client(*this, connection, content_check)
   {
   }

   template<typename F> void transaction(F transaction)
   {
    Client::transaction([this, &transaction](Client_Data &data)
    {
     transaction(get_database(), get_multiplexer());
    });
   }
 };
}

#endif
