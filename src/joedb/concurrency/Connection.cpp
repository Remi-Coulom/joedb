#include "joedb/concurrency/Connection.h"

namespace joedb
{
 void Readonly_Connection::content_mismatch()
 {
  throw Exception("Client data does not match the server");
 }

 int64_t Readonly_Connection::handshake(Readonly_Journal &client_journal)
 {
  return client_journal.get_checkpoint_position();
 }

 int64_t Readonly_Connection::pull(Writable_Journal &client_journal)
 {
  client_journal.pull();
  return client_journal.get_checkpoint_position();
 }

 Readonly_Connection::~Readonly_Connection() = default;

 int64_t Connection::lock_pull(Writable_Journal &client_journal)
 {
  client_journal.lock_pull();
  return client_journal.get_checkpoint_position();
 }

 int64_t Connection::push
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

 void Connection::unlock(Readonly_Journal &client_journal)
 {
  client_journal.unlock();
 }
}
