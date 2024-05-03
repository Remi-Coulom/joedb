#ifndef joedb_Connection_declared
#define joedb_Connection_declared

#include "joedb/journal/Writable_Journal.h"

namespace joedb
{
 class Connection;

 ////////////////////////////////////////////////////////////////////////////
 class Pullonly_Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   static void content_mismatch();

  public:
   virtual int64_t handshake(Readonly_Journal &client_journal);
   virtual int64_t pull(Writable_Journal &client_journal);
   virtual ~Pullonly_Connection();
   virtual Connection *get_push_connection();
 };

 ////////////////////////////////////////////////////////////////////////////
 class Connection: public virtual Pullonly_Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   virtual int64_t lock_pull(Writable_Journal &client_journal);

   virtual int64_t push
   (
    Readonly_Journal &client_journal,
    int64_t server_checkpoint,
    bool unlock_after
   );

   virtual void unlock(Readonly_Journal &client_journal);

   virtual Connection *get_push_connection() final;
 };
}

#endif
