#include "joedb/concurrency/Connection.h"

namespace joedb
{
 void Connection::content_mismatch()
 {
  throw Exception("Content mismatch. The file and the connection have diverged, and cannot be synced by pulling or pushing.");
 }

 int64_t Connection::handshake
 (
  const Readonly_Journal &client_journal,
  bool content_check
 )
 {
  return client_journal.get_checkpoint_position();
 }

 int64_t Connection::pull
 (
  bool lock_before,
  Writable_Journal *client_journal,
  std::chrono::milliseconds wait
 )
 {
  if (client_journal)
   return client_journal->get_checkpoint_position();
  else
   return -1;
 }

 int64_t Connection::push
 (
  const Readonly_Journal &client_journal,
  int64_t from,
  int64_t until,
  bool unlock_after
 )
 {
  return client_journal.get_checkpoint_position();
 }

 void Connection::unlock()
 {
 }

 Connection::~Connection() = default;
}
