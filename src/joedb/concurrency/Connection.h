#ifndef joedb_Connection_declared
#define joedb_Connection_declared

#include "joedb/journal/Writable_Journal.h"
#include "joedb/concurrency/Mutex.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Connection: public Mutex
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Client;
  friend class Connection_Write_Lock;

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
 class Connection_Write_Lock
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Connection &connection;
   Writable_Journal &journal;
   const int64_t server_position;

  public:
   Connection_Write_Lock
   (
    Connection &connection,
    Writable_Journal &journal
   ):
    connection(connection),
    journal(journal),
    server_position(connection.lock_pull(journal))
   {
   }

   ~Connection_Write_Lock() noexcept(false)
   {
    try
    {
     journal.checkpoint(0);
     connection.push_unlock(journal, server_position);
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
