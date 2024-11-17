#ifndef joedb_Connection_declared
#define joedb_Connection_declared

#include "joedb/journal/Writable_Journal.h"
#include <limits>

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
   virtual int64_t handshake
   (
    Readonly_Journal &client_journal,
    bool content_check
   );
   virtual int64_t pull(Writable_Journal &client_journal);
   virtual Connection *get_push_connection();
   virtual ~Pullonly_Connection();
 };

 ////////////////////////////////////////////////////////////////////////////
 class Connection: public virtual Pullonly_Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   virtual int64_t lock_pull(Writable_Journal &client_journal);

   virtual int64_t push_until
   (
    Readonly_Journal &client_journal,
    int64_t from_checkpoint,
    int64_t until_checkpoint,
    bool unlock_after
   );

   int64_t push
   (
    Readonly_Journal &client_journal,
    int64_t from_checkpoint,
    bool unlock_after
   )
   {
    return push_until
    (
     client_journal,
     from_checkpoint,
     std::numeric_limits<int64_t>::max(),
     unlock_after
    );
   }

   virtual void unlock(Readonly_Journal &client_journal);

   virtual Connection *get_push_connection();
 };
}

#endif
