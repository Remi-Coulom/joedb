#ifndef joedb_Connection_declared
#define joedb_Connection_declared

#include "joedb/journal/Writable_Journal.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 class Connection_Puller
 ////////////////////////////////////////////////////////////////////////////
 {
  public:
   virtual int64_t pull(Writable_Journal &client_journal);
   virtual ~Connection_Puller();
 };

 ////////////////////////////////////////////////////////////////////////////
 class Connection_Pusher
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
   virtual ~Connection_Pusher();
 };

 ////////////////////////////////////////////////////////////////////////////
 class Connection
 ////////////////////////////////////////////////////////////////////////////
 {
  protected:
   static void content_mismatch();

  public:
   virtual int64_t handshake(Readonly_Journal &client_journal);

   virtual Connection_Puller *get_puller();
   virtual Connection_Pusher *get_pusher();

   virtual ~Connection();
 };

 ////////////////////////////////////////////////////////////////////////////
 class Pull_Connection:
 ////////////////////////////////////////////////////////////////////////////
  public Connection_Puller,
  public Connection
 {
  public:
   Connection_Puller *get_puller() override;
 };

 ////////////////////////////////////////////////////////////////////////////
 class Push_Connection:
 ////////////////////////////////////////////////////////////////////////////
  public Connection_Pusher,
  public Connection
 {
  public:
   Connection_Pusher *get_pusher() override;
 };

 ////////////////////////////////////////////////////////////////////////////
 class Full_Connection:
 ////////////////////////////////////////////////////////////////////////////
  public Connection_Puller,
  public Connection_Pusher,
  public Connection
 {
  public:
   Connection_Puller *get_puller() override;
   Connection_Pusher *get_pusher() override;
 };

 using Local_Connection = Full_Connection;
}

#endif
