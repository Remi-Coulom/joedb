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
   virtual void pull(Journal_File &client_journal) = 0;
   virtual void lock_pull(Journal_File &client_journal) = 0;
   virtual void push_unlock(Readonly_Journal &client_journal) = 0;

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

   void lock_pull()
   {
    connection.lock_pull(journal);
    journal.play_until_checkpoint(writable);
   }

   void push_unlock()
   {
    journal.checkpoint(0);
    connection.push_unlock(journal);
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
   }

   void pull()
   {
    connection.pull(journal);
    journal.play_until_checkpoint(writable);
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

   ~Lock()
   {
    try
    {
     control.push_unlock();
    }
    catch (...)
    {
    }
   }
 };
}

#endif
