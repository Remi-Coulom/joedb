#ifndef joedb_Server_declared
#define joedb_Server_declared

#include "joedb/journal/Journal_File.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Server
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Server_Access;
  friend class Write_Lock;

  private:
   virtual void lock() {}
   virtual void unlock() {}

   virtual void pull(Journal_File &client_journal) {}
   virtual void push(Readonly_Journal &client_journal) {}

  public:
   virtual ~Server() {}
 };

 ////////////////////////////////////////////////////////////////////////////
 class Server_Access
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Write_Lock;

  private:
   Server &server;
   Journal_File &journal;
   Writable &writable;

  public:
   Server_Access
   (
    Server &server,
    Journal_File &journal,
    Writable &writable
   ):
    server(server),
    journal(journal),
    writable(writable)
   {
   }

   void pull()
   {
    server.pull(journal);
    journal.play_until_checkpoint(writable);
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Write_Lock
 ////////////////////////////////////////////////////////////////////////////
 {
  private:
   Server_Access &access;

  public:
   Write_Lock(Server_Access &access): access(access)
   {
    access.server.lock();
    access.pull();
   }

   ~Write_Lock()
   {
    access.journal.checkpoint(0);
    access.server.push(access.journal);
    access.server.unlock();
   }
 };
}

#endif
