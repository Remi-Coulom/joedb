#include "joedb/concurrency/Connection.h"

namespace joedb
{
 ////////////////////////////////////////////////////////////////////////////
 int64_t Connection_Puller::pull(Writable_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  client_journal.pull();
  return client_journal.get_checkpoint_position();
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Connection_Pusher::lock_pull(Writable_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  client_journal.lock_pull();
  return client_journal.get_checkpoint_position();
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Connection_Pusher::push
 ////////////////////////////////////////////////////////////////////////////
 (
  Readonly_Journal &client_journal,
  int64_t server_checkpoint,
  bool unlock_after
 )
 {
  if (unlock_after)
   client_journal.unlock();
  return client_journal.get_checkpoint_position();
 }

 ////////////////////////////////////////////////////////////////////////////
 void Connection_Pusher::unlock(Readonly_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  client_journal.unlock();
 }

 ////////////////////////////////////////////////////////////////////////////
 void Connection::content_mismatch()
 ////////////////////////////////////////////////////////////////////////////
 {
  throw Exception("Client data does not match the server");
 }

 ////////////////////////////////////////////////////////////////////////////
 int64_t Connection::handshake(Readonly_Journal &client_journal)
 ////////////////////////////////////////////////////////////////////////////
 {
  return client_journal.get_checkpoint_position();
 }

 Connection_Puller::~Connection_Puller() = default;
 Connection_Pusher::~Connection_Pusher() = default;
 Connection::~Connection() = default;

 Connection_Puller *Connection::get_puller() {return nullptr;}
 Connection_Pusher *Connection::get_pusher() {return nullptr;}
 Connection_Puller *Pull_Connection::get_puller() {return this;}
 Connection_Pusher *Push_Connection::get_pusher() {return this;}
 Connection_Puller *Full_Connection::get_puller() {return this;}
 Connection_Pusher *Full_Connection::get_pusher() {return this;}
}
