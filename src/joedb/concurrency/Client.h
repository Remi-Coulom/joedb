#ifndef joedb_Client_declared
#define joedb_Client_declared

#include "joedb/concurrency/Connection.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Client
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Client_Write_Lock;

  private:
   Connection &connection;
   Writable_Journal &journal;
   Writable &writable;

   int64_t server_position;

   void lock_pull()
   {
    server_position = connection.lock_pull(journal);
    journal.play_until_checkpoint(writable);
   }

   void push_unlock()
   {
    journal.checkpoint(0);
    connection.push_unlock(journal, server_position);
   }

  public:
   Client
   (
    Connection &connection,
    Writable_Journal &journal,
    Writable &writable
   ):
    connection(connection),
    journal(journal),
    writable(writable)
   {
    journal.play_until_checkpoint(writable);
   }

   int64_t pull()
   {
    server_position = connection.pull(journal);
    journal.play_until_checkpoint(writable);
    return server_position;
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Client_Write_Lock
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Client &client;

  public:
   Client_Write_Lock(Client &client): client(client)
   {
    client.lock_pull();
   }

   ~Client_Write_Lock() noexcept(false)
   {
    try
    {
     client.push_unlock();
    }
    catch (...)
    {
     if (!std::uncaught_exception())
      throw;
    }
   }
 };
}

#endif
