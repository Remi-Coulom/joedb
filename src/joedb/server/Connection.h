#ifndef joedb_Connection_declared
#define joedb_Connection_declared

#include "joedb/journal/Journal_File.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Connection_Control;

  private:
   virtual int64_t pull(Journal_File &client_journal) = 0;

   virtual int64_t lock_pull(Journal_File &client_journal) = 0;

   virtual void push_unlock
   (
    Readonly_Journal &client_journal,
    int64_t server_position
   ) = 0;

  public:
   virtual ~Connection() {}
 };

 ////////////////////////////////////////////////////////////////////////////
 class Connection_Control
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Lock;

  private:
   Connection &connection;
   Journal_File &journal;
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
   Connection_Control
   (
    Connection &connection,
    Journal_File &journal,
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
   Connection_Control &control;

  public:
   Lock(Connection_Control &control): control(control)
   {
    control.lock_pull();
   }

   ~Lock() noexcept(false)
   {
    try
    {
     control.push_unlock();
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
