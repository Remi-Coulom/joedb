#ifndef joedb_Backup_Client_declared
#define joedb_Backup_Client_declared

#include "joedb/concurrency/Client.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Backup_Client_Data
 ////////////////////////////////////////////////////////////////////////////
 {
  friend class Backup_Client;

  private:
   Writable_Journal journal;

  public:
   Backup_Client_Data
   (
    joedb::Connection &connection,
    Generic_File &file
   ):
    journal(file)
   {
   }

   Writable_Journal &get_journal()
   {
    return journal;
   }

   const Writable_Journal &get_journal() const
   {
    return journal;
   }

   void update()
   {
   }
 };

 ////////////////////////////////////////////////////////////////////////////
 class Backup_Client: public Client<Backup_Client_Data>
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   Backup_Client
   (
    Connection &connection,
    Generic_File &local_file
   ):
    Client(connection, local_file)
   {
   }

  Writable_Journal &get_journal()
  {
   return data.journal;
  }
 };
}

#endif
