#include "joedb/concurrency/Connection.h"

namespace joedb
{
 void Connection::content_mismatch()
 {
  throw Exception("Content mismatch. The file and the connection have diverged, and cannot be synced by pulling or pushing.");
 }

 int64_t Connection::handshake
 (
  Readonly_Journal &client_journal,
  bool content_check
 )
 {
  return client_journal.get_checkpoint_position();
 }

 int64_t Connection::pull
 (
  Writable_Journal &client_journal,
  std::chrono::milliseconds wait
 )
 {
  client_journal.pull();
  return client_journal.get_checkpoint_position();
 }

 int64_t Connection::lock_pull
 (
  Writable_Journal &client_journal,
  std::chrono::milliseconds wait
 )
 {
  client_journal.lock_pull();
  return client_journal.get_checkpoint_position();
 }

 int64_t Connection::push_until
 (
  Readonly_Journal &client_journal,
  int64_t from_checkpoint,
  int64_t until_checkpoint,
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

 bool Connection::is_pullonly() const
 {
  return false;
 }

 Connection::~Connection() = default;
}
