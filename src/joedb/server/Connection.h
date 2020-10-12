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
  friend class Write_Lock;

  private:
   virtual void lock() {}
   virtual void unlock() {}

   virtual void pull(Journal_File &client_journal) {}
   virtual void push(Readonly_Journal &client_journal) {}

  public:
   virtual ~Connection() {}
 };

 ////////////////////////////////////////////////////////////////////////////
 class Connection_Control
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Write_Lock;

  private:
   Connection &connection;
   Journal_File &journal;
   Writable &writable;

   void write_lock()
   {
    connection.lock();
    connection.pull(journal);
    journal.play_until_checkpoint(writable);
   }

   void write_unlock()
   {
    journal.checkpoint(0);
    connection.push(journal);
    connection.unlock();
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
    connection.lock();
    connection.pull(journal);
    connection.unlock();
    journal.play_until_checkpoint(writable);
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Write_Lock
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Connection_Control &control;

  public:
   Write_Lock(Connection_Control &control): control(control)
   {
    control.write_lock();
   }

   ~Write_Lock()
   {
    control.write_unlock();
   }
 };
}

#endif
