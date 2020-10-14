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
   virtual void lock_pull_unlock(Journal_File &client_journal) {}
   virtual void lock_pull(Journal_File &client_journal) {}
   virtual void push_unlock(Readonly_Journal &client_journal) {}

  public:
   virtual ~Connection() {}
 };

 ////////////////////////////////////////////////////////////////////////////
 class Decomposed_Connection: public Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   virtual void lock() {}
   virtual void unlock() {}

   virtual void pull(Journal_File &client_journal) {}
   virtual void push(Readonly_Journal &client_journal) {}

   void lock_pull_unlock(Journal_File &client_journal) override
   {
    lock();
    pull(client_journal);
    unlock();
   }

   void lock_pull(Journal_File &client_journal) override
   {
    lock();
    pull(client_journal);
   }

   void push_unlock(Readonly_Journal &client_journal) override
   {
    push(client_journal);
    unlock();
   }
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

   void lock()
   {
    connection.lock_pull(journal);
    journal.play_until_checkpoint(writable);
   }

   void unlock()
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
    connection.lock_pull_unlock(journal);
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
    control.lock();
   }

   ~Lock()
   {
    control.unlock();
   }
 };
}

#endif
