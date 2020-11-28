#ifndef joedb_Connection_declared
#define joedb_Connection_declared

#include "joedb/journal/Writable_Journal.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Client;

  private:
   virtual int64_t pull(Writable_Journal &client_journal) = 0;

   virtual int64_t lock_pull(Writable_Journal &client_journal) = 0;

   virtual void push_unlock
   (
    Readonly_Journal &client_journal,
    int64_t server_position
   ) = 0;

  public:
   virtual ~Connection() {}
 };

 ////////////////////////////////////////////////////////////////////////////
 class Client
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Lock;
  friend class Lockless_Push;

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
 class Lock
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Client &client;

  public:
   Lock(Client &client): client(client)
   {
    client.lock_pull();
   }

   ~Lock() noexcept(false)
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